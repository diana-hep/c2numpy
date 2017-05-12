#include <assert.h>
#include <pthread.h>
#include <inttypes.h>
#include <typeinfo>
#include <string>

#include <iostream>

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

  inline void wait(uint64_t forstate) {
    while (pthread_rwlock_rdlock(statelock) != 0) sleep(1);
    uint64_t current = state;
    pthread_rwlock_unlock(statelock);

    while (current != forstate) {
      sleep(1);
      while (pthread_rwlock_rdlock(statelock) != 0) sleep(1);
      current = state;
      pthread_rwlock_unlock(statelock);
    }
  }

  inline void waitmask(uint64_t formask) {
    while (pthread_rwlock_rdlock(statelock) != 0) sleep(1);
    uint64_t current = state;
    pthread_rwlock_unlock(statelock);

    while (current & formask) {
      sleep(1);
      while (pthread_rwlock_rdlock(statelock) != 0) sleep(1);
      current = state;
      pthread_rwlock_unlock(statelock);
    }
  }

  inline void notify(uint64_t newstate) {
    while (pthread_rwlock_wrlock(statelock) != 0) sleep(1);
    state = newstate;
    pthread_rwlock_unlock(statelock);
  }

  void checkall() {
    std::cout << "numArrays " << numArrays << std::endl;
    std::cout << "names (" << names[0] << ") (" << names[1] << ") (" << names[2] << ")" << std::endl;
    std::cout << "types (" << types[0] << ") (" << types[1] << ") (" << types[2] << ")" << std::endl;
    std::cout << "data " << ((double**)data)[0][0] << " " << ((double**)data)[1][0] << " " << ((double**)data)[2][0] << std::endl;
    std::cout << "lengths " << lengths[0] << " " << lengths[1] << " " << lengths[2] << std::endl;
    std::cout << "state " << state << std::endl;

    std::cout << "lock0 " << pthread_rwlock_wrlock(locks[0]) << std::endl;
    std::cout << "lock1 " << pthread_rwlock_wrlock(locks[1]) << std::endl;
    std::cout << "lock2 " << pthread_rwlock_wrlock(locks[2]) << std::endl;
    std::cout << "lockstate " << pthread_rwlock_wrlock(statelock) << std::endl;

    std::cout << "unlockstate " << pthread_rwlock_unlock(statelock) << std::endl;
    std::cout << "unlock2 " << pthread_rwlock_unlock(locks[2]) << std::endl;
    std::cout << "unlock1 " << pthread_rwlock_unlock(locks[1]) << std::endl;
    std::cout << "unlock0 " << pthread_rwlock_unlock(locks[0]) << std::endl;

    std::cout << "lock0 " << pthread_rwlock_rdlock(locks[0]) << std::endl;
    std::cout << "lock1 " << pthread_rwlock_rdlock(locks[1]) << std::endl;
    std::cout << "lock2 " << pthread_rwlock_rdlock(locks[2]) << std::endl;
    std::cout << "lockstate " << pthread_rwlock_rdlock(statelock) << std::endl;

    std::cout << "unlockstate " << pthread_rwlock_unlock(statelock) << std::endl;
    std::cout << "unlock2 " << pthread_rwlock_unlock(locks[2]) << std::endl;
    std::cout << "unlock1 " << pthread_rwlock_unlock(locks[1]) << std::endl;
    std::cout << "unlock0 " << pthread_rwlock_unlock(locks[0]) << std::endl;

    std::cout << "YAY" << std::endl;
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

  inline const T get(uint64_t index) {
    assert(index < length);
    return data[index];
  }

  inline const void set(uint64_t index, T value) {
    data[index] = value;
  }

  inline T safeget(uint64_t index) {
    assert(index < length);
    while (pthread_rwlock_rdlock(lock) != 0) sleep(1);
    T out = data[index];
    pthread_rwlock_unlock(lock);
    return out;
  }

  inline void safeset(uint64_t index, T value) {
    assert(index < length);
    while (pthread_rwlock_wrlock(lock) != 0) sleep(1);
    data[index] = value;
    pthread_rwlock_unlock(lock);
  }

private:
  pthread_rwlock_t *lock;
  T *data;
  uint64_t length;
};
