/**
 * Autogenerated by Thrift Compiler (0.13.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef rpc_TYPES_H
#define rpc_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <functional>
#include <memory>


namespace rocksdb {

class TCompactFilesRequest;

class TFileDescriptor;

class TFileMetadata;

class TStatus;

class TCompactionResult;

class TDeletedCompactionInput;

class TInstalledCompactionOutput;

class TInstallCompactionRequest;

class GetResult;

typedef struct _TCompactFilesRequest__isset {
  _TCompactFilesRequest__isset() : cf_name(false), flush_nums(false), compaction_nums(false), path_ids(false), output_level(false), start_file_num(false), max_file_num(false), comp_start(false), comp_end(false) {}
  bool cf_name :1;
  bool flush_nums :1;
  bool compaction_nums :1;
  bool path_ids :1;
  bool output_level :1;
  bool start_file_num :1;
  bool max_file_num :1;
  bool comp_start :1;
  bool comp_end :1;
} _TCompactFilesRequest__isset;

class TCompactFilesRequest : public virtual ::apache::thrift::TBase {
 public:

  TCompactFilesRequest(const TCompactFilesRequest&);
  TCompactFilesRequest& operator=(const TCompactFilesRequest&);
  TCompactFilesRequest() : cf_name(), output_level(0), start_file_num(0), max_file_num(0), comp_start(), comp_end() {
  }

  virtual ~TCompactFilesRequest() noexcept;
  std::string cf_name;
  std::vector<int64_t>  flush_nums;
  std::vector<int64_t>  compaction_nums;
  std::vector<int32_t>  path_ids;
  int32_t output_level;
  int64_t start_file_num;
  int32_t max_file_num;
  std::string comp_start;
  std::string comp_end;

  _TCompactFilesRequest__isset __isset;

  void __set_cf_name(const std::string& val);

  void __set_flush_nums(const std::vector<int64_t> & val);

  void __set_compaction_nums(const std::vector<int64_t> & val);

  void __set_path_ids(const std::vector<int32_t> & val);

  void __set_output_level(const int32_t val);

  void __set_start_file_num(const int64_t val);

  void __set_max_file_num(const int32_t val);

  void __set_comp_start(const std::string& val);

  void __set_comp_end(const std::string& val);

  bool operator == (const TCompactFilesRequest & rhs) const
  {
    if (!(cf_name == rhs.cf_name))
      return false;
    if (!(flush_nums == rhs.flush_nums))
      return false;
    if (!(compaction_nums == rhs.compaction_nums))
      return false;
    if (!(path_ids == rhs.path_ids))
      return false;
    if (!(output_level == rhs.output_level))
      return false;
    if (!(start_file_num == rhs.start_file_num))
      return false;
    if (!(max_file_num == rhs.max_file_num))
      return false;
    if (!(comp_start == rhs.comp_start))
      return false;
    if (!(comp_end == rhs.comp_end))
      return false;
    return true;
  }
  bool operator != (const TCompactFilesRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCompactFilesRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TCompactFilesRequest &a, TCompactFilesRequest &b);

std::ostream& operator<<(std::ostream& out, const TCompactFilesRequest& obj);

typedef struct _TFileDescriptor__isset {
  _TFileDescriptor__isset() : flush_number(false), merge_number(false), path_id(false), file_size(false), smallest_seqno(false), largest_seqno(false) {}
  bool flush_number :1;
  bool merge_number :1;
  bool path_id :1;
  bool file_size :1;
  bool smallest_seqno :1;
  bool largest_seqno :1;
} _TFileDescriptor__isset;

class TFileDescriptor : public virtual ::apache::thrift::TBase {
 public:

  TFileDescriptor(const TFileDescriptor&);
  TFileDescriptor& operator=(const TFileDescriptor&);
  TFileDescriptor() : flush_number(0), merge_number(0), path_id(0), file_size(0), smallest_seqno(0), largest_seqno(0) {
  }

  virtual ~TFileDescriptor() noexcept;
  int64_t flush_number;
  int64_t merge_number;
  int32_t path_id;
  int64_t file_size;
  int64_t smallest_seqno;
  int64_t largest_seqno;

  _TFileDescriptor__isset __isset;

  void __set_flush_number(const int64_t val);

  void __set_merge_number(const int64_t val);

  void __set_path_id(const int32_t val);

  void __set_file_size(const int64_t val);

  void __set_smallest_seqno(const int64_t val);

  void __set_largest_seqno(const int64_t val);

  bool operator == (const TFileDescriptor & rhs) const
  {
    if (!(flush_number == rhs.flush_number))
      return false;
    if (!(merge_number == rhs.merge_number))
      return false;
    if (!(path_id == rhs.path_id))
      return false;
    if (!(file_size == rhs.file_size))
      return false;
    if (!(smallest_seqno == rhs.smallest_seqno))
      return false;
    if (!(largest_seqno == rhs.largest_seqno))
      return false;
    return true;
  }
  bool operator != (const TFileDescriptor &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TFileDescriptor & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TFileDescriptor &a, TFileDescriptor &b);

std::ostream& operator<<(std::ostream& out, const TFileDescriptor& obj);

typedef struct _TFileMetadata__isset {
  _TFileMetadata__isset() : fd(false), smallest_key(false), largest_key(false), num_entries(false), num_deletions(false), raw_key_size(false), raw_value_size(false), oldest_ancester_time(false), file_creation_time(false), file_checksum(false), file_checksum_func_name(false) {}
  bool fd :1;
  bool smallest_key :1;
  bool largest_key :1;
  bool num_entries :1;
  bool num_deletions :1;
  bool raw_key_size :1;
  bool raw_value_size :1;
  bool oldest_ancester_time :1;
  bool file_creation_time :1;
  bool file_checksum :1;
  bool file_checksum_func_name :1;
} _TFileMetadata__isset;

class TFileMetadata : public virtual ::apache::thrift::TBase {
 public:

  TFileMetadata(const TFileMetadata&);
  TFileMetadata& operator=(const TFileMetadata&);
  TFileMetadata() : smallest_key(), largest_key(), num_entries(0), num_deletions(0), raw_key_size(0), raw_value_size(0), oldest_ancester_time(0), file_creation_time(0), file_checksum(), file_checksum_func_name() {
  }

  virtual ~TFileMetadata() noexcept;
  TFileDescriptor fd;
  std::string smallest_key;
  std::string largest_key;
  int64_t num_entries;
  int64_t num_deletions;
  int64_t raw_key_size;
  int64_t raw_value_size;
  int64_t oldest_ancester_time;
  int64_t file_creation_time;
  std::string file_checksum;
  std::string file_checksum_func_name;

  _TFileMetadata__isset __isset;

  void __set_fd(const TFileDescriptor& val);

  void __set_smallest_key(const std::string& val);

  void __set_largest_key(const std::string& val);

  void __set_num_entries(const int64_t val);

  void __set_num_deletions(const int64_t val);

  void __set_raw_key_size(const int64_t val);

  void __set_raw_value_size(const int64_t val);

  void __set_oldest_ancester_time(const int64_t val);

  void __set_file_creation_time(const int64_t val);

  void __set_file_checksum(const std::string& val);

  void __set_file_checksum_func_name(const std::string& val);

  bool operator == (const TFileMetadata & rhs) const
  {
    if (!(fd == rhs.fd))
      return false;
    if (!(smallest_key == rhs.smallest_key))
      return false;
    if (!(largest_key == rhs.largest_key))
      return false;
    if (!(num_entries == rhs.num_entries))
      return false;
    if (!(num_deletions == rhs.num_deletions))
      return false;
    if (!(raw_key_size == rhs.raw_key_size))
      return false;
    if (!(raw_value_size == rhs.raw_value_size))
      return false;
    if (!(oldest_ancester_time == rhs.oldest_ancester_time))
      return false;
    if (!(file_creation_time == rhs.file_creation_time))
      return false;
    if (!(file_checksum == rhs.file_checksum))
      return false;
    if (!(file_checksum_func_name == rhs.file_checksum_func_name))
      return false;
    return true;
  }
  bool operator != (const TFileMetadata &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TFileMetadata & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TFileMetadata &a, TFileMetadata &b);

std::ostream& operator<<(std::ostream& out, const TFileMetadata& obj);

typedef struct _TStatus__isset {
  _TStatus__isset() : code(false), sub_code(false), severity(false), state(false) {}
  bool code :1;
  bool sub_code :1;
  bool severity :1;
  bool state :1;
} _TStatus__isset;

class TStatus : public virtual ::apache::thrift::TBase {
 public:

  TStatus(const TStatus&);
  TStatus& operator=(const TStatus&);
  TStatus() : code(0), sub_code(0), severity(0), state() {
  }

  virtual ~TStatus() noexcept;
  int32_t code;
  int32_t sub_code;
  int32_t severity;
  std::string state;

  _TStatus__isset __isset;

  void __set_code(const int32_t val);

  void __set_sub_code(const int32_t val);

  void __set_severity(const int32_t val);

  void __set_state(const std::string& val);

  bool operator == (const TStatus & rhs) const
  {
    if (!(code == rhs.code))
      return false;
    if (!(sub_code == rhs.sub_code))
      return false;
    if (!(severity == rhs.severity))
      return false;
    if (!(state == rhs.state))
      return false;
    return true;
  }
  bool operator != (const TStatus &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TStatus & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TStatus &a, TStatus &b);

std::ostream& operator<<(std::ostream& out, const TStatus& obj);

typedef struct _TCompactionResult__isset {
  _TCompactionResult__isset() : status(false), output_files(false), total_bytes(false), num_output_records(false), db_paths(false) {}
  bool status :1;
  bool output_files :1;
  bool total_bytes :1;
  bool num_output_records :1;
  bool db_paths :1;
} _TCompactionResult__isset;

class TCompactionResult : public virtual ::apache::thrift::TBase {
 public:

  TCompactionResult(const TCompactionResult&);
  TCompactionResult& operator=(const TCompactionResult&);
  TCompactionResult() : total_bytes(0), num_output_records(0) {
  }

  virtual ~TCompactionResult() noexcept;
  TStatus status;
  std::vector<TFileMetadata>  output_files;
  int64_t total_bytes;
  int64_t num_output_records;
  std::vector<std::string>  db_paths;

  _TCompactionResult__isset __isset;

  void __set_status(const TStatus& val);

  void __set_output_files(const std::vector<TFileMetadata> & val);

  void __set_total_bytes(const int64_t val);

  void __set_num_output_records(const int64_t val);

  void __set_db_paths(const std::vector<std::string> & val);

  bool operator == (const TCompactionResult & rhs) const
  {
    if (!(status == rhs.status))
      return false;
    if (!(output_files == rhs.output_files))
      return false;
    if (!(total_bytes == rhs.total_bytes))
      return false;
    if (!(num_output_records == rhs.num_output_records))
      return false;
    if (!(db_paths == rhs.db_paths))
      return false;
    return true;
  }
  bool operator != (const TCompactionResult &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCompactionResult & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TCompactionResult &a, TCompactionResult &b);

std::ostream& operator<<(std::ostream& out, const TCompactionResult& obj);

typedef struct _TDeletedCompactionInput__isset {
  _TDeletedCompactionInput__isset() : level(false), flush_num(false), compaction_num(false) {}
  bool level :1;
  bool flush_num :1;
  bool compaction_num :1;
} _TDeletedCompactionInput__isset;

class TDeletedCompactionInput : public virtual ::apache::thrift::TBase {
 public:

  TDeletedCompactionInput(const TDeletedCompactionInput&);
  TDeletedCompactionInput& operator=(const TDeletedCompactionInput&);
  TDeletedCompactionInput() : level(0), flush_num(0), compaction_num(0) {
  }

  virtual ~TDeletedCompactionInput() noexcept;
  int32_t level;
  int64_t flush_num;
  int64_t compaction_num;

  _TDeletedCompactionInput__isset __isset;

  void __set_level(const int32_t val);

  void __set_flush_num(const int64_t val);

  void __set_compaction_num(const int64_t val);

  bool operator == (const TDeletedCompactionInput & rhs) const
  {
    if (!(level == rhs.level))
      return false;
    if (!(flush_num == rhs.flush_num))
      return false;
    if (!(compaction_num == rhs.compaction_num))
      return false;
    return true;
  }
  bool operator != (const TDeletedCompactionInput &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TDeletedCompactionInput & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TDeletedCompactionInput &a, TDeletedCompactionInput &b);

std::ostream& operator<<(std::ostream& out, const TDeletedCompactionInput& obj);

typedef struct _TInstalledCompactionOutput__isset {
  _TInstalledCompactionOutput__isset() : level(false), metadata(false) {}
  bool level :1;
  bool metadata :1;
} _TInstalledCompactionOutput__isset;

class TInstalledCompactionOutput : public virtual ::apache::thrift::TBase {
 public:

  TInstalledCompactionOutput(const TInstalledCompactionOutput&);
  TInstalledCompactionOutput& operator=(const TInstalledCompactionOutput&);
  TInstalledCompactionOutput() : level(0) {
  }

  virtual ~TInstalledCompactionOutput() noexcept;
  int32_t level;
  TFileMetadata metadata;

  _TInstalledCompactionOutput__isset __isset;

  void __set_level(const int32_t val);

  void __set_metadata(const TFileMetadata& val);

  bool operator == (const TInstalledCompactionOutput & rhs) const
  {
    if (!(level == rhs.level))
      return false;
    if (!(metadata == rhs.metadata))
      return false;
    return true;
  }
  bool operator != (const TInstalledCompactionOutput &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TInstalledCompactionOutput & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TInstalledCompactionOutput &a, TInstalledCompactionOutput &b);

std::ostream& operator<<(std::ostream& out, const TInstalledCompactionOutput& obj);

typedef struct _TInstallCompactionRequest__isset {
  _TInstallCompactionRequest__isset() : deleted_inputs(false), installed_outputs(false) {}
  bool deleted_inputs :1;
  bool installed_outputs :1;
} _TInstallCompactionRequest__isset;

class TInstallCompactionRequest : public virtual ::apache::thrift::TBase {
 public:

  TInstallCompactionRequest(const TInstallCompactionRequest&);
  TInstallCompactionRequest& operator=(const TInstallCompactionRequest&);
  TInstallCompactionRequest() {
  }

  virtual ~TInstallCompactionRequest() noexcept;
  std::vector<TDeletedCompactionInput>  deleted_inputs;
  std::vector<TInstalledCompactionOutput>  installed_outputs;

  _TInstallCompactionRequest__isset __isset;

  void __set_deleted_inputs(const std::vector<TDeletedCompactionInput> & val);

  void __set_installed_outputs(const std::vector<TInstalledCompactionOutput> & val);

  bool operator == (const TInstallCompactionRequest & rhs) const
  {
    if (!(deleted_inputs == rhs.deleted_inputs))
      return false;
    if (!(installed_outputs == rhs.installed_outputs))
      return false;
    return true;
  }
  bool operator != (const TInstallCompactionRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TInstallCompactionRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(TInstallCompactionRequest &a, TInstallCompactionRequest &b);

std::ostream& operator<<(std::ostream& out, const TInstallCompactionRequest& obj);

typedef struct _GetResult__isset {
  _GetResult__isset() : status(false), value(false) {}
  bool status :1;
  bool value :1;
} _GetResult__isset;

class GetResult : public virtual ::apache::thrift::TBase {
 public:

  GetResult(const GetResult&);
  GetResult& operator=(const GetResult&);
  GetResult() : value() {
  }

  virtual ~GetResult() noexcept;
  TStatus status;
  std::string value;

  _GetResult__isset __isset;

  void __set_status(const TStatus& val);

  void __set_value(const std::string& val);

  bool operator == (const GetResult & rhs) const
  {
    if (!(status == rhs.status))
      return false;
    if (!(value == rhs.value))
      return false;
    return true;
  }
  bool operator != (const GetResult &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const GetResult & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(GetResult &a, GetResult &b);

std::ostream& operator<<(std::ostream& out, const GetResult& obj);

} // namespace

#endif
