#ifndef RAY_ID_H_
#define RAY_ID_H_

#include <inttypes.h>

#include <cstring>
#include <string>

#include "plasma/common.h"
#include "ray/constants.h"
#include "ray/util/visibility.h"

namespace ray {

class RAY_EXPORT UniqueID {
 public:
  UniqueID();
  UniqueID(const plasma::UniqueID &from);
  static UniqueID from_random();
  static UniqueID from_binary(const std::string &binary);
  static const UniqueID &nil();
  size_t hash() const;
  bool is_nil() const;
  bool operator==(const UniqueID &rhs) const;
  bool operator!=(const UniqueID &rhs) const;
  const uint8_t *data() const;
  static size_t size();
  std::string binary() const;
  std::string hex() const;
  plasma::UniqueID to_plasma_id() const;

 private:
  UniqueID(const std::string &binary);

 protected:
  uint8_t id_[kUniqueIDSize];
  mutable size_t hash_ = 0;
};

static_assert(std::is_standard_layout<UniqueID>::value, "UniqueID must be standard");

std::ostream &operator<<(std::ostream &os, const UniqueID &id);

#define DEFINE_UNIQUE_ID(type)                                                  \
  class RAY_EXPORT type : public UniqueID {                                     \
   public:                                                                      \
    explicit type(const UniqueID &from) {                                       \
      std::memcpy(&id_, from.data(), kUniqueIDSize);                            \
    }                                                                           \
    type() : UniqueID() {}                                                      \
    static type from_random() { return type(UniqueID::from_random()); }         \
    static type from_binary(const std::string &binary) { return type(binary); } \
    static type nil() { return type(UniqueID::nil()); }                         \
    static size_t size() { return kUniqueIDSize; }                              \
                                                                                \
   private:                                                                     \
    explicit type(const std::string &binary) {                                  \
      std::memcpy(&id_, binary.data(), kUniqueIDSize);                          \
    }                                                                           \
  };

#include "id_def.h"

#undef DEFINE_UNIQUE_ID

// TODO(swang): ObjectID and TaskID should derive from UniqueID. Then, we
// can make these methods of the derived classes.
/// Finish computing a task ID. Since objects created by the task share a
/// prefix of the ID, the suffix of the task ID is zeroed out by this function.
///
/// \param task_id A task ID to finish.
/// \return The finished task ID. It may now be used to compute IDs for objects
/// created by the task.
const TaskID FinishTaskId(const TaskID &task_id);

/// Compute the object ID of an object returned by the task.
///
/// \param task_id The task ID of the task that created the object.
/// \param return_index What number return value this object is in the task.
/// \return The computed object ID.
const ObjectID ComputeReturnId(const TaskID &task_id, int64_t return_index);

/// Compute the object ID of an object put by the task.
///
/// \param task_id The task ID of the task that created the object.
/// \param put_index What number put this object was created by in the task.
/// \return The computed object ID.
const ObjectID ComputePutId(const TaskID &task_id, int64_t put_index);

/// Compute the task ID of the task that created the object.
///
/// \param object_id The object ID.
/// \return The task ID of the task that created this object.
const TaskID ComputeTaskId(const ObjectID &object_id);

/// Generate a task ID from the given info.
///
/// \param driver_id The driver that creates the task.
/// \param parent_task_id The parent task of this task.
/// \param parent_task_counter The task index of the worker.
/// \return The task ID generated from the given info.
const TaskID GenerateTaskId(const DriverID &driver_id, const TaskID &parent_task_id,
                            int parent_task_counter);

/// Compute the index of this object in the task that created it.
///
/// \param object_id The object ID.
/// \return The index of object creation according to the task that created
/// this object. This is positive if the task returned the object and negative
/// if created by a put.
int64_t ComputeObjectIndex(const ObjectID &object_id);

}  // namespace ray

namespace std {

#define DEFINE_UNIQUE_ID(type)                                           \
  template <>                                                            \
  struct hash<::ray::type> {                                             \
    size_t operator()(const ::ray::type &id) const { return id.hash(); } \
  };                                                                     \
  template <>                                                            \
  struct hash<const ::ray::type> {                                       \
    size_t operator()(const ::ray::type &id) const { return id.hash(); } \
  };

DEFINE_UNIQUE_ID(UniqueID);
#include "id_def.h"

#undef DEFINE_UNIQUE_ID
}  // namespace std
#endif  // RAY_ID_H_
