namespace cpp rocksdb

struct TCompactFilesRequest {
    1: string cf_name
    2: list<i64> file_nums
    3: i32 output_level
    4: i64 start_file_num
    5: i32 max_file_num
    6: binary comp_start
    7: binary comp_end
}

struct TFileDescriptor {
    1: i64 packed_number_and_path_id
    2: i64 file_size
    3: i64 smallest_seqno
    4: i64 largest_seqno
}

struct TFileMetadata {
    1: TFileDescriptor fd
    2: binary smallest_key
    3: binary largest_key
    4: i64 num_entries
    5: i64 num_deletions
    6: i64 raw_key_size
    7: i64 raw_value_size
    8: i64 oldest_ancester_time
    9: i64 file_creation_time
    10: string file_checksum
    11: string file_checksum_func_name
}

struct TStatus {
    1: i32 code
    2: i32 sub_code
    3: i32 severity
    4: string state
}

struct TCompactionResult {
    1: TStatus status
    2: list<TFileMetadata> output_files
    3: i64 total_bytes
    4: i64 num_output_records
    5: list<string> db_paths
}

struct TDeletedCompactionInput {
    1: i32 level
    2: i64 file_num
}

struct TInstalledCompactionOutput {
    1: i32 level
    2: TFileMetadata metadata
}

struct TInstallCompactionRequest {
    1: list<TDeletedCompactionInput> deleted_inputs
    2: list<TInstalledCompactionOutput> installed_outputs
}

struct GetResult {
   1: TStatus status
   2: string value
}

service ThriftService {
    TCompactionResult CompactFiles(1: TCompactFilesRequest request)
    binary DownLoadFile(1: string file_name, 2: i64 offset, 3: i32 size)
    void PushFiles(1: TCompactionResult output_files, 2: string source_ip, 3: i32 source_port)
    void SetFileNumber(1: i64 new_file_num)
    TStatus InstallCompaction(1: TInstallCompactionRequest request);
    TStatus Put(1: string key, 2: string value)
    GetResult Get(1: string key)
    TStatus FullCompaction()
    TStatus Flush()
}