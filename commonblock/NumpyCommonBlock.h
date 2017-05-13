// Copyright 2017 Jim Pivarski
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <assert.h>
#include <pthread.h>
#include <inttypes.h>
#include <typeinfo>
#include <string>

template <typename T> class NumpyCommonBlockAccessor;

class NumpyCommonBlock {
  template <typename> friend class NumpyCommonBlockAccessor;

public:
  template <typename T> NumpyCommonBlockAccessor<T> accessor(std::string name) {
    uint64_t which = 0;
    while (which < numArrays) {
      if (std::string(names[which]) == name)
        break;
      which++;
    };
    assert(which < numArrays);

    return NumpyCommonBlockAccessor<T>(this, which);
  }

  template <typename T> NumpyCommonBlockAccessor<T>* newAccessor(std::string name) {
    uint64_t which = 0;
    while (which < numArrays) {
      if (std::string(names[which]) == name)
        break;
      which++;
    };
    assert(which < numArrays);

    return new NumpyCommonBlockAccessor<T>(this, which);
  }

  inline void wait(uint64_t forstate) {
    while (pthread_rwlock_rdlock(statelock) != 0) usleep(1);
    uint64_t current = state;
    pthread_rwlock_unlock(statelock);

    while (current != forstate) {
      usleep(1);
      while (pthread_rwlock_rdlock(statelock) != 0) usleep(1);
      current = state;
      pthread_rwlock_unlock(statelock);
    }
  }

  inline void waitmask(uint64_t formask) {
    while (pthread_rwlock_rdlock(statelock) != 0) usleep(1);
    uint64_t current = state;
    pthread_rwlock_unlock(statelock);

    while (!(current & formask)) {
      usleep(1);
      while (pthread_rwlock_rdlock(statelock) != 0) usleep(1);
      current = state;
      pthread_rwlock_unlock(statelock);
    }
  }

  inline void notify(uint64_t newstate) {
    while (pthread_rwlock_wrlock(statelock) != 0) usleep(1);
    state = newstate;
    pthread_rwlock_unlock(statelock);
  }

private:
  NumpyCommonBlock() { }   // can't create them
  uint64_t numArrays;
  char **names;
  char **types;
  void **data;
  uint64_t *lengths;
  pthread_rwlock_t **locks;
  pthread_rwlock_t *statelock;
  uint64_t state;
};

template <typename T> class NumpyCommonBlockAccessor {
public:
  NumpyCommonBlockAccessor(NumpyCommonBlock *cb, uint64_t which) {
    std::string type = std::string(cb->types[which]);

    if (type == std::string("bool"))
      assert(typeid(T) == typeid(bool));

    else if (type == std::string("int8"))
      assert(typeid(T) == typeid(int8_t));

    else if (type == std::string("uint8"))
      assert(typeid(T) == typeid(uint8_t));

    else if (type == std::string("int16"))
      assert(typeid(T) == typeid(int16_t));

    else if (type == std::string("uint16"))
      assert(typeid(T) == typeid(uint16_t));

    else if (type == std::string("int32"))
      assert(typeid(T) == typeid(int32_t));

    else if (type == std::string("uint32"))
      assert(typeid(T) == typeid(uint32_t));

    else if (type == std::string("int64"))
      assert(typeid(T) == typeid(int64_t));

    else if (type == std::string("uint64"))
      assert(typeid(T) == typeid(uint64_t));

    else if (type == std::string("float32"))
      assert(typeid(T) == typeid(float));

    else if (type == std::string("float64"))
      assert(typeid(T) == typeid(double));

    else
      assert(false);

    lock = cb->locks[which];
    data = (T*)(cb->data[which]);
    length = cb->lengths[which];
  }

  inline T get(uint64_t index) {
    assert(index < length);
    return data[index];
  }

  inline void set(uint64_t index, T value) {
    data[index] = value;
  }

  inline T safeget(uint64_t index) {
    assert(index < length);
    while (pthread_rwlock_rdlock(lock) != 0) usleep(1);
    T out = data[index];
    pthread_rwlock_unlock(lock);
    return out;
  }

  inline void safeset(uint64_t index, T value) {
    assert(index < length);
    while (pthread_rwlock_wrlock(lock) != 0) usleep(1);
    data[index] = value;
    pthread_rwlock_unlock(lock);
  }

  inline uint64_t size() {
    return length;
  }

private:
  pthread_rwlock_t *lock;
  T *data;
  uint64_t length;
};
