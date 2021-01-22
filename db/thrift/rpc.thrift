namespace cpp rocksdb

struct TCompactFilesRequest {
    1: string cf_name
    2: list<i64> flush_nums
    3: list<i64> compaction_nums
    4: list<i32> path_ids
    5: i32 output_level
    6: i64 start_file_num
    7: i32 max_file_num
    8: binary comp_start
    9: binary comp_end
}

struct TFileDescriptor {
    1: i64 flush_number
    2: i64 merge_number
    3: i32 path_id
    4: i64 file_size
    5: i64 smallest_seqno
    6: i64 largest_seqno
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
    2: i64 flush_num
    3: i64 compaction_num
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
    void UpLoadTableFile(1: string file_name, 2: binary data, 3: bool is_last, 4: i32 path_num)
    void PushFiles(1: TCompactionResult output_files, 2: string source_ip, 3: i32 source_port)
    void SetCompactionNumber(1: i64 new_compaction_num)
    TStatus InstallCompaction(1: TInstallCompactionRequest request);
    TStatus Put(1: string key, 2: string value)
    TStatus PutBatch(1: list<string> key, 2: list<string> value)
    GetResult Get(1: string key)
    TStatus FullCompaction()
    TStatus Flush()
}