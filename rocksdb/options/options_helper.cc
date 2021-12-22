//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
#include "options/options_helper.h"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <unordered_set>
#include <vector>

#include "options/options_type.h"
#include "rocksdb/cache.h"
#include "rocksdb/compaction_filter.h"
#include "rocksdb/convenience.h"
#include "rocksdb/filter_policy.h"
#include "rocksdb/memtablerep.h"
#include "rocksdb/merge_operator.h"
#include "rocksdb/options.h"
#include "rocksdb/rate_limiter.h"
#include "rocksdb/slice_transform.h"
#include "rocksdb/table.h"
#include "rocksdb/utilities/object_registry.h"
#include "table/block_based/block_based_table_factory.h"
#include "table/plain/plain_table_factory.h"
#include "util/string_util.h"

namespace ROCKSDB_NAMESPACE {

DBOptions BuildDBOptions(const ImmutableDBOptions& immutable_db_options,
                         const MutableDBOptions& mutable_db_options) {
  DBOptions options;

  options.create_if_missing = immutable_db_options.create_if_missing;
  options.create_missing_column_families =
      immutable_db_options.create_missing_column_families;
  options.error_if_exists = immutable_db_options.error_if_exists;
  options.paranoid_checks = immutable_db_options.paranoid_checks;
  options.env = immutable_db_options.env;
  options.rate_limiter = immutable_db_options.rate_limiter;
  options.sst_file_manager = immutable_db_options.sst_file_manager;
  options.info_log = immutable_db_options.info_log;
  options.info_log_level = immutable_db_options.info_log_level;
  options.max_open_files = mutable_db_options.max_open_files;
  options.max_file_opening_threads =
      immutable_db_options.max_file_opening_threads;
  options.max_total_wal_size = mutable_db_options.max_total_wal_size;
  options.statistics = immutable_db_options.statistics;
  options.use_fsync = immutable_db_options.use_fsync;
  options.db_paths = immutable_db_options.db_paths;
  options.db_log_dir = immutable_db_options.db_log_dir;
  options.wal_dir = immutable_db_options.wal_dir;
  options.delete_obsolete_files_period_micros =
      mutable_db_options.delete_obsolete_files_period_micros;
  options.max_background_jobs = mutable_db_options.max_background_jobs;
  options.base_background_compactions =
      mutable_db_options.base_background_compactions;
  options.max_background_compactions =
      mutable_db_options.max_background_compactions;
  options.bytes_per_sync = mutable_db_options.bytes_per_sync;
  options.wal_bytes_per_sync = mutable_db_options.wal_bytes_per_sync;
  options.strict_bytes_per_sync = mutable_db_options.strict_bytes_per_sync;
  options.max_subcompactions = immutable_db_options.max_subcompactions;
  options.max_background_flushes = mutable_db_options.max_background_flushes;
  options.max_log_file_size = immutable_db_options.max_log_file_size;
  options.log_file_time_to_roll = immutable_db_options.log_file_time_to_roll;
  options.keep_log_file_num = immutable_db_options.keep_log_file_num;
  options.recycle_log_file_num = immutable_db_options.recycle_log_file_num;
  options.max_manifest_file_size = immutable_db_options.max_manifest_file_size;
  options.table_cache_numshardbits =
      immutable_db_options.table_cache_numshardbits;
  options.WAL_ttl_seconds = immutable_db_options.wal_ttl_seconds;
  options.WAL_size_limit_MB = immutable_db_options.wal_size_limit_mb;
  options.manifest_preallocation_size =
      immutable_db_options.manifest_preallocation_size;
  options.allow_mmap_reads = immutable_db_options.allow_mmap_reads;
  options.allow_mmap_writes = immutable_db_options.allow_mmap_writes;
  options.use_direct_reads = immutable_db_options.use_direct_reads;
  options.use_direct_io_for_flush_and_compaction =
      immutable_db_options.use_direct_io_for_flush_and_compaction;
  options.allow_fallocate = immutable_db_options.allow_fallocate;
  options.is_fd_close_on_exec = immutable_db_options.is_fd_close_on_exec;
  options.stats_dump_period_sec = mutable_db_options.stats_dump_period_sec;
  options.stats_persist_period_sec =
      mutable_db_options.stats_persist_period_sec;
  options.persist_stats_to_disk = immutable_db_options.persist_stats_to_disk;
  options.stats_history_buffer_size =
      mutable_db_options.stats_history_buffer_size;
  options.advise_random_on_open = immutable_db_options.advise_random_on_open;
  options.db_write_buffer_size = immutable_db_options.db_write_buffer_size;
  options.write_buffer_manager = immutable_db_options.write_buffer_manager;
  options.access_hint_on_compaction_start =
      immutable_db_options.access_hint_on_compaction_start;
  options.new_table_reader_for_compaction_inputs =
      immutable_db_options.new_table_reader_for_compaction_inputs;
  options.compaction_readahead_size =
      mutable_db_options.compaction_readahead_size;
  options.random_access_max_buffer_size =
      immutable_db_options.random_access_max_buffer_size;
  options.writable_file_max_buffer_size =
      mutable_db_options.writable_file_max_buffer_size;
  options.use_adaptive_mutex = immutable_db_options.use_adaptive_mutex;
  options.listeners = immutable_db_options.listeners;
  options.enable_thread_tracking = immutable_db_options.enable_thread_tracking;
  options.delayed_write_rate = mutable_db_options.delayed_write_rate;
  options.enable_pipelined_write = immutable_db_options.enable_pipelined_write;
  options.unordered_write = immutable_db_options.unordered_write;
  options.allow_concurrent_memtable_write =
      immutable_db_options.allow_concurrent_memtable_write;
  options.enable_write_thread_adaptive_yield =
      immutable_db_options.enable_write_thread_adaptive_yield;
  options.max_write_batch_group_size_bytes =
      immutable_db_options.max_write_batch_group_size_bytes;
  options.write_thread_max_yield_usec =
      immutable_db_options.write_thread_max_yield_usec;
  options.write_thread_slow_yield_usec =
      immutable_db_options.write_thread_slow_yield_usec;
  options.skip_stats_update_on_db_open =
      immutable_db_options.skip_stats_update_on_db_open;
  options.skip_checking_sst_file_sizes_on_db_open =
      immutable_db_options.skip_checking_sst_file_sizes_on_db_open;
  options.wal_recovery_mode = immutable_db_options.wal_recovery_mode;
  options.allow_2pc = immutable_db_options.allow_2pc;
  options.row_cache = immutable_db_options.row_cache;
#ifndef ROCKSDB_LITE
  options.wal_filter = immutable_db_options.wal_filter;
#endif  // ROCKSDB_LITE
  options.fail_if_options_file_error =
      immutable_db_options.fail_if_options_file_error;
  options.dump_malloc_stats = immutable_db_options.dump_malloc_stats;
  options.avoid_flush_during_recovery =
      immutable_db_options.avoid_flush_during_recovery;
  options.avoid_flush_during_shutdown =
      mutable_db_options.avoid_flush_during_shutdown;
  options.allow_ingest_behind =
      immutable_db_options.allow_ingest_behind;
  options.preserve_deletes =
      immutable_db_options.preserve_deletes;
  options.two_write_queues = immutable_db_options.two_write_queues;
  options.manual_wal_flush = immutable_db_options.manual_wal_flush;
  options.atomic_flush = immutable_db_options.atomic_flush;
  options.avoid_unnecessary_blocking_io =
      immutable_db_options.avoid_unnecessary_blocking_io;
  options.log_readahead_size = immutable_db_options.log_readahead_size;
  options.file_checksum_gen_factory =
      immutable_db_options.file_checksum_gen_factory;
  options.best_efforts_recovery = immutable_db_options.best_efforts_recovery;
  return options;
}

ColumnFamilyOptions BuildColumnFamilyOptions(
    const ColumnFamilyOptions& options,
    const MutableCFOptions& mutable_cf_options) {
  ColumnFamilyOptions cf_opts(options);

  // Memtable related options
  cf_opts.write_buffer_size = mutable_cf_options.write_buffer_size;
  cf_opts.max_write_buffer_number = mutable_cf_options.max_write_buffer_number;
  cf_opts.arena_block_size = mutable_cf_options.arena_block_size;
  cf_opts.memtable_prefix_bloom_size_ratio =
      mutable_cf_options.memtable_prefix_bloom_size_ratio;
  cf_opts.memtable_whole_key_filtering =
      mutable_cf_options.memtable_whole_key_filtering;
  cf_opts.memtable_huge_page_size = mutable_cf_options.memtable_huge_page_size;
  cf_opts.max_successive_merges = mutable_cf_options.max_successive_merges;
  cf_opts.inplace_update_num_locks =
      mutable_cf_options.inplace_update_num_locks;
  cf_opts.prefix_extractor = mutable_cf_options.prefix_extractor;

  // Compaction related options
  cf_opts.disable_auto_compactions =
      mutable_cf_options.disable_auto_compactions;
  cf_opts.soft_pending_compaction_bytes_limit =
      mutable_cf_options.soft_pending_compaction_bytes_limit;
  cf_opts.hard_pending_compaction_bytes_limit =
      mutable_cf_options.hard_pending_compaction_bytes_limit;
  cf_opts.level0_file_num_compaction_trigger =
      mutable_cf_options.level0_file_num_compaction_trigger;
  cf_opts.level0_slowdown_writes_trigger =
      mutable_cf_options.level0_slowdown_writes_trigger;
  cf_opts.level0_stop_writes_trigger =
      mutable_cf_options.level0_stop_writes_trigger;
  cf_opts.max_compaction_bytes = mutable_cf_options.max_compaction_bytes;
  cf_opts.target_file_size_base = mutable_cf_options.target_file_size_base;
  cf_opts.target_file_size_multiplier =
      mutable_cf_options.target_file_size_multiplier;
  cf_opts.max_bytes_for_level_base =
      mutable_cf_options.max_bytes_for_level_base;
  cf_opts.max_bytes_for_level_multiplier =
      mutable_cf_options.max_bytes_for_level_multiplier;
  cf_opts.ttl = mutable_cf_options.ttl;
  cf_opts.periodic_compaction_seconds =
      mutable_cf_options.periodic_compaction_seconds;

  cf_opts.max_bytes_for_level_multiplier_additional.clear();
  for (auto value :
       mutable_cf_options.max_bytes_for_level_multiplier_additional) {
    cf_opts.max_bytes_for_level_multiplier_additional.emplace_back(value);
  }

  cf_opts.compaction_options_fifo = mutable_cf_options.compaction_options_fifo;
  cf_opts.compaction_options_universal =
      mutable_cf_options.compaction_options_universal;

  // Misc options
  cf_opts.max_sequential_skip_in_iterations =
      mutable_cf_options.max_sequential_skip_in_iterations;
  cf_opts.paranoid_file_checks = mutable_cf_options.paranoid_file_checks;
  cf_opts.report_bg_io_stats = mutable_cf_options.report_bg_io_stats;
  cf_opts.compression = mutable_cf_options.compression;
  cf_opts.compression_opts = mutable_cf_options.compression_opts;
  cf_opts.bottommost_compression = mutable_cf_options.bottommost_compression;
  cf_opts.bottommost_compression_opts =
      mutable_cf_options.bottommost_compression_opts;
  cf_opts.sample_for_compression = mutable_cf_options.sample_for_compression;

  cf_opts.table_factory = options.table_factory;
  // TODO(yhchiang): find some way to handle the following derived options
  // * max_file_size

  return cf_opts;
}

std::map<CompactionStyle, std::string>
    OptionsHelper::compaction_style_to_string = {
        {kCompactionStyleLevel, "kCompactionStyleLevel"},
        {kCompactionStyleUniversal, "kCompactionStyleUniversal"},
        {kCompactionStyleFIFO, "kCompactionStyleFIFO"},
        {kCompactionStyleNone, "kCompactionStyleNone"}};

std::map<CompactionPri, std::string> OptionsHelper::compaction_pri_to_string = {
    {kByCompensatedSize, "kByCompensatedSize"},
    {kOldestLargestSeqFirst, "kOldestLargestSeqFirst"},
    {kOldestSmallestSeqFirst, "kOldestSmallestSeqFirst"},
    {kMinOverlappingRatio, "kMinOverlappingRatio"}};

std::map<CompactionStopStyle, std::string>
    OptionsHelper::compaction_stop_style_to_string = {
        {kCompactionStopStyleSimilarSize, "kCompactionStopStyleSimilarSize"},
        {kCompactionStopStyleTotalSize, "kCompactionStopStyleTotalSize"}};

std::unordered_map<std::string, ChecksumType>
    OptionsHelper::checksum_type_string_map = {{"kNoChecksum", kNoChecksum},
                                               {"kCRC32c", kCRC32c},
                                               {"kxxHash", kxxHash},
                                               {"kxxHash64", kxxHash64}};

std::unordered_map<std::string, CompressionType>
    OptionsHelper::compression_type_string_map = {
        {"kNoCompression", kNoCompression},
        {"kSnappyCompression", kSnappyCompression},
        {"kZlibCompression", kZlibCompression},
        {"kBZip2Compression", kBZip2Compression},
        {"kLZ4Compression", kLZ4Compression},
        {"kLZ4HCCompression", kLZ4HCCompression},
        {"kXpressCompression", kXpressCompression},
        {"kZSTD", kZSTD},
        {"kZSTDNotFinalCompression", kZSTDNotFinalCompression},
        {"kDisableCompressionOption", kDisableCompressionOption}};

std::vector<CompressionType> GetSupportedCompressions() {
  std::vector<CompressionType> supported_compressions;
  for (const auto& comp_to_name : OptionsHelper::compression_type_string_map) {
    CompressionType t = comp_to_name.second;
    if (t != kDisableCompressionOption && CompressionTypeSupported(t)) {
      supported_compressions.push_back(t);
    }
  }
  return supported_compressions;
}

#ifndef ROCKSDB_LITE

namespace {
template <typename T>
bool ParseEnum(const std::unordered_map<std::string, T>& type_map,
               const std::string& type, T* value) {
  auto iter = type_map.find(type);
  if (iter != type_map.end()) {
    *value = iter->second;
    return true;
  }
  return false;
}

template <typename T>
bool SerializeEnum(const std::unordered_map<std::string, T>& type_map,
                   const T& type, std::string* value) {
  for (const auto& pair : type_map) {
    if (pair.second == type) {
      *value = pair.first;
      return true;
    }
  }
  return false;
}

bool SerializeVectorCompressionType(const std::vector<CompressionType>& types,
                                    std::string* value) {
  std::stringstream ss;
  bool result;
  for (size_t i = 0; i < types.size(); ++i) {
    if (i > 0) {
      ss << ':';
    }
    std::string string_type;
    result = SerializeEnum<CompressionType>(compression_type_string_map,
                                            types[i], &string_type);
    if (result == false) {
      return result;
    }
    ss << string_type;
  }
  *value = ss.str();
  return true;
}

bool ParseVectorCompressionType(
    const std::string& value,
    std::vector<CompressionType>* compression_per_level) {
  compression_per_level->clear();
  size_t start = 0;
  while (start < value.size()) {
    size_t end = value.find(':', start);
    bool is_ok;
    CompressionType type;
    if (end == std::string::npos) {
      is_ok = ParseEnum<CompressionType>(compression_type_string_map,
                                         value.substr(start), &type);
      if (!is_ok) {
        return false;
      }
      compression_per_level->emplace_back(type);
      break;
    } else {
      is_ok = ParseEnum<CompressionType>(
          compression_type_string_map, value.substr(start, end - start), &type);
      if (!is_ok) {
        return false;
      }
      compression_per_level->emplace_back(type);
      start = end + 1;
    }
  }
  return true;
}

// This is to handle backward compatibility, where compaction_options_fifo
// could be assigned a single scalar value, say, like "23", which would be
// assigned to max_table_files_size.
bool FIFOCompactionOptionsSpecialCase(const std::string& opt_str,
                                      CompactionOptionsFIFO* options) {
  if (opt_str.find("=") != std::string::npos) {
    // New format. Go do your new parsing using ParseStructOptions.
    return false;
  }

  // Old format. Parse just a single uint64_t value.
  options->max_table_files_size = ParseUint64(opt_str);
  return true;
}

static bool SerializeStruct(
    const void* const opt_ptr, std::string* value,
    const std::unordered_map<std::string, OptionTypeInfo>& type_info_map) {
  ConfigOptions config_options;
  config_options.delimiter = ";";

  std::string opt_str;
  Status s =
      GetStringFromStruct(config_options, opt_ptr, type_info_map, &opt_str);
  if (!s.ok()) {
    return false;
  }
  *value = "{" + opt_str + "}";
  return true;
}

static bool ParseSingleStructOption(
    const ConfigOptions& config_options, const std::string& opt_val_str,
    void* options,
    const std::unordered_map<std::string, OptionTypeInfo>& type_info_map) {
  size_t end = opt_val_str.find('=');
  std::string key = opt_val_str.substr(0, end);
  std::string value = opt_val_str.substr(end + 1);
  auto iter = type_info_map.find(key);
  if (iter == type_info_map.end()) {
    return false;
  }
  const auto& opt_info = iter->second;
  Status s = opt_info.ParseOption(
      config_options, key, value,
      reinterpret_cast<char*>(options) + opt_info.mutable_offset);
  return s.ok();
}

static bool ParseStructOptions(
    const std::string& opt_str, void* options,
    const std::unordered_map<std::string, OptionTypeInfo>& type_info_map) {
  assert(!opt_str.empty());
  ConfigOptions config_options;

  size_t start = 0;
  if (opt_str[0] == '{') {
    start++;
  }
  while ((start != std::string::npos) && (start < opt_str.size())) {
    if (opt_str[start] == '}') {
      break;
    }
    size_t end = opt_str.find(';', start);
    size_t len = (end == std::string::npos) ? end : end - start;
    if (!ParseSingleStructOption(config_options, opt_str.substr(start, len),
                                 options, type_info_map)) {
      return false;
    }
    start = (end == std::string::npos) ? end : end + 1;
  }
  return true;
}
}  // anonymouse namespace

bool ParseSliceTransformHelper(
    const std::string& kFixedPrefixName, const std::string& kCappedPrefixName,
    const std::string& value,
    std::shared_ptr<const SliceTransform>* slice_transform) {
  const char* no_op_name = "rocksdb.Noop";
  size_t no_op_length = strlen(no_op_name);
  auto& pe_value = value;
  if (pe_value.size() > kFixedPrefixName.size() &&
      pe_value.compare(0, kFixedPrefixName.size(), kFixedPrefixName) == 0) {
    int prefix_length = ParseInt(trim(value.substr(kFixedPrefixName.size())));
    slice_transform->reset(NewFixedPrefixTransform(prefix_length));
  } else if (pe_value.size() > kCappedPrefixName.size() &&
             pe_value.compare(0, kCappedPrefixName.size(), kCappedPrefixName) ==
                 0) {
    int prefix_length =
        ParseInt(trim(pe_value.substr(kCappedPrefixName.size())));
    slice_transform->reset(NewCappedPrefixTransform(prefix_length));
  } else if (pe_value.size() == no_op_length &&
             pe_value.compare(0, no_op_length, no_op_name) == 0) {
    const SliceTransform* no_op_transform = NewNoopTransform();
    slice_transform->reset(no_op_transform);
  } else if (value == kNullptrString) {
    slice_transform->reset();
  } else {
    return false;
  }

  return true;
}

bool ParseSliceTransform(
    const std::string& value,
    std::shared_ptr<const SliceTransform>* slice_transform) {
  // While we normally don't convert the string representation of a
  // pointer-typed option into its instance, here we do so for backward
  // compatibility as we allow this action in SetOption().

  // TODO(yhchiang): A possible better place for these serialization /
  // deserialization is inside the class definition of pointer-typed
  // option itself, but this requires a bigger change of public API.
  bool result =
      ParseSliceTransformHelper("fixed:", "capped:", value, slice_transform);
  if (result) {
    return result;
  }
  result = ParseSliceTransformHelper(
      "rocksdb.FixedPrefix.", "rocksdb.CappedPrefix.", value, slice_transform);
  if (result) {
    return result;
  }
  // TODO(yhchiang): we can further support other default
  //                 SliceTransforms here.
  return false;
}

bool ParseOptionHelper(char* opt_address, const OptionType& opt_type,
                       const std::string& value) {
  switch (opt_type) {
    case OptionType::kBoolean:
      *reinterpret_cast<bool*>(opt_address) = ParseBoolean("", value);
      break;
    case OptionType::kInt:
      *reinterpret_cast<int*>(opt_address) = ParseInt(value);
      break;
    case OptionType::kInt32T:
      *reinterpret_cast<int32_t*>(opt_address) = ParseInt32(value);
      break;
    case OptionType::kInt64T:
      PutUnaligned(reinterpret_cast<int64_t*>(opt_address), ParseInt64(value));
      break;
    case OptionType::kVectorInt:
      *reinterpret_cast<std::vector<int>*>(opt_address) = ParseVectorInt(value);
      break;
    case OptionType::kUInt:
      *reinterpret_cast<unsigned int*>(opt_address) = ParseUint32(value);
      break;
    case OptionType::kUInt32T:
      *reinterpret_cast<uint32_t*>(opt_address) = ParseUint32(value);
      break;
    case OptionType::kUInt64T:
      PutUnaligned(reinterpret_cast<uint64_t*>(opt_address), ParseUint64(value));
      break;
    case OptionType::kSizeT:
      PutUnaligned(reinterpret_cast<size_t*>(opt_address), ParseSizeT(value));
      break;
    case OptionType::kString:
      *reinterpret_cast<std::string*>(opt_address) = value;
      break;
    case OptionType::kDouble:
      *reinterpret_cast<double*>(opt_address) = ParseDouble(value);
      break;
    case OptionType::kCompactionStyle:
      return ParseEnum<CompactionStyle>(
          compaction_style_string_map, value,
          reinterpret_cast<CompactionStyle*>(opt_address));
    case OptionType::kCompactionPri:
      return ParseEnum<CompactionPri>(
          compaction_pri_string_map, value,
          reinterpret_cast<CompactionPri*>(opt_address));
    case OptionType::kCompressionType:
      return ParseEnum<CompressionType>(
          compression_type_string_map, value,
          reinterpret_cast<CompressionType*>(opt_address));
    case OptionType::kVectorCompressionType:
      return ParseVectorCompressionType(
          value, reinterpret_cast<std::vector<CompressionType>*>(opt_address));
    case OptionType::kSliceTransform:
      return ParseSliceTransform(
          value, reinterpret_cast<std::shared_ptr<const SliceTransform>*>(
                     opt_address));
    case OptionType::kChecksumType:
      return ParseEnum<ChecksumType>(
          checksum_type_string_map, value,
          reinterpret_cast<ChecksumType*>(opt_address));
    case OptionType::kBlockBasedTableIndexType:
      return ParseEnum<BlockBasedTableOptions::IndexType>(
          block_base_table_index_type_string_map, value,
          reinterpret_cast<BlockBasedTableOptions::IndexType*>(opt_address));
    case OptionType::kBlockBasedTableDataBlockIndexType:
      return ParseEnum<BlockBasedTableOptions::DataBlockIndexType>(
          block_base_table_data_block_index_type_string_map, value,
          reinterpret_cast<BlockBasedTableOptions::DataBlockIndexType*>(
              opt_address));
    case OptionType::kBlockBasedTableIndexShorteningMode:
      return ParseEnum<BlockBasedTableOptions::IndexShorteningMode>(
          block_base_table_index_shortening_mode_string_map, value,
          reinterpret_cast<BlockBasedTableOptions::IndexShorteningMode*>(
              opt_address));
    case OptionType::kEncodingType:
      return ParseEnum<EncodingType>(
          encoding_type_string_map, value,
          reinterpret_cast<EncodingType*>(opt_address));
    case OptionType::kWALRecoveryMode:
      return ParseEnum<WALRecoveryMode>(
          wal_recovery_mode_string_map, value,
          reinterpret_cast<WALRecoveryMode*>(opt_address));
    case OptionType::kAccessHint:
      return ParseEnum<DBOptions::AccessHint>(
          access_hint_string_map, value,
          reinterpret_cast<DBOptions::AccessHint*>(opt_address));
    case OptionType::kInfoLogLevel:
      return ParseEnum<InfoLogLevel>(
          info_log_level_string_map, value,
          reinterpret_cast<InfoLogLevel*>(opt_address));
    case OptionType::kCompactionOptionsFIFO: {
      if (!FIFOCompactionOptionsSpecialCase(
              value, reinterpret_cast<CompactionOptionsFIFO*>(opt_address))) {
        return ParseStructOptions(value, opt_address,
                                  fifo_compaction_options_type_info);
      }
      return true;
    }
    case OptionType::kLRUCacheOptions: {
      return ParseStructOptions(value, opt_address,
                                lru_cache_options_type_info);
    }
    case OptionType::kCompactionOptionsUniversal:
      return ParseStructOptions(value, opt_address,
                                universal_compaction_options_type_info);
    case OptionType::kCompactionStopStyle:
      return ParseEnum<CompactionStopStyle>(
          compaction_stop_style_string_map, value,
          reinterpret_cast<CompactionStopStyle*>(opt_address));
    default:
      return false;
  }
  return true;
}

bool SerializeSingleOptionHelper(const char* opt_address,
                                 const OptionType opt_type,
                                 std::string* value) {

  assert(value);
  switch (opt_type) {
    case OptionType::kBoolean:
      *value = *(reinterpret_cast<const bool*>(opt_address)) ? "true" : "false";
      break;
    case OptionType::kInt:
      *value = ToString(*(reinterpret_cast<const int*>(opt_address)));
      break;
    case OptionType::kInt32T:
      *value = ToString(*(reinterpret_cast<const int32_t*>(opt_address)));
      break;
    case OptionType::kInt64T:
      {
        int64_t v;
        GetUnaligned(reinterpret_cast<const int64_t*>(opt_address), &v);
        *value = ToString(v);
      }
      break;
    case OptionType::kVectorInt:
      return SerializeIntVector(
          *reinterpret_cast<const std::vector<int>*>(opt_address), value);
    case OptionType::kUInt:
      *value = ToString(*(reinterpret_cast<const unsigned int*>(opt_address)));
      break;
    case OptionType::kUInt32T:
      *value = ToString(*(reinterpret_cast<const uint32_t*>(opt_address)));
      break;
    case OptionType::kUInt64T:
      {
        uint64_t v;
        GetUnaligned(reinterpret_cast<const uint64_t*>(opt_address), &v);
        *value = ToString(v);
      }
      break;
    case OptionType::kSizeT:
      {
        size_t v;
        GetUnaligned(reinterpret_cast<const size_t*>(opt_address), &v);
        *value = ToString(v);
      }
      break;
    case OptionType::kDouble:
      *value = ToString(*(reinterpret_cast<const double*>(opt_address)));
      break;
    case OptionType::kString:
      *value = EscapeOptionString(
          *(reinterpret_cast<const std::string*>(opt_address)));
      break;
    case OptionType::kCompactionStyle:
      return SerializeEnum<CompactionStyle>(
          compaction_style_string_map,
          *(reinterpret_cast<const CompactionStyle*>(opt_address)), value);
    case OptionType::kCompactionPri:
      return SerializeEnum<CompactionPri>(
          compaction_pri_string_map,
          *(reinterpret_cast<const CompactionPri*>(opt_address)), value);
    case OptionType::kCompressionType:
      return SerializeEnum<CompressionType>(
          compression_type_string_map,
          *(reinterpret_cast<const CompressionType*>(opt_address)), value);
    case OptionType::kVectorCompressionType:
      return SerializeVectorCompressionType(
          *(reinterpret_cast<const std::vector<CompressionType>*>(opt_address)),
          value);
      break;
    case OptionType::kSliceTransform: {
      const auto* slice_transform_ptr =
          reinterpret_cast<const std::shared_ptr<const SliceTransform>*>(
              opt_address);
      *value = slice_transform_ptr->get() ? slice_transform_ptr->get()->Name()
                                          : kNullptrString;
      break;
    }
    case OptionType::kTableFactory: {
      const auto* table_factory_ptr =
          reinterpret_cast<const std::shared_ptr<const TableFactory>*>(
              opt_address);
      *value = table_factory_ptr->get() ? table_factory_ptr->get()->Name()
                                        : kNullptrString;
      break;
    }
    case OptionType::kComparator: {
      // it's a const pointer of const Comparator*
      const auto* ptr = reinterpret_cast<const Comparator* const*>(opt_address);
      // Since the user-specified comparator will be wrapped by
      // InternalKeyComparator, we should persist the user-specified one
      // instead of InternalKeyComparator.
      if (*ptr == nullptr) {
        *value = kNullptrString;
      } else {
        const Comparator* root_comp = (*ptr)->GetRootComparator();
        if (root_comp == nullptr) {
          root_comp = (*ptr);
        }
        *value = root_comp->Name();
      }
      break;
    }
    case OptionType::kCompactionFilter: {
      // it's a const pointer of const CompactionFilter*
      const auto* ptr =
          reinterpret_cast<const CompactionFilter* const*>(opt_address);
      *value = *ptr ? (*ptr)->Name() : kNullptrString;
      break;
    }
    case OptionType::kCompactionFilterFactory: {
      const auto* ptr =
          reinterpret_cast<const std::shared_ptr<CompactionFilterFactory>*>(
              opt_address);
      *value = ptr->get() ? ptr->get()->Name() : kNullptrString;
      break;
    }
    case OptionType::kMemTableRepFactory: {
      const auto* ptr =
          reinterpret_cast<const std::shared_ptr<MemTableRepFactory>*>(
              opt_address);
      *value = ptr->get() ? ptr->get()->Name() : kNullptrString;
      break;
    }
    case OptionType::kMergeOperator: {
      const auto* ptr =
          reinterpret_cast<const std::shared_ptr<MergeOperator>*>(opt_address);
      *value = ptr->get() ? ptr->get()->Name() : kNullptrString;
      break;
    }
    case OptionType::kFilterPolicy: {
      const auto* ptr =
          reinterpret_cast<const std::shared_ptr<FilterPolicy>*>(opt_address);
      *value = ptr->get() ? ptr->get()->Name() : kNullptrString;
      break;
    }
    case OptionType::kChecksumType:
      return SerializeEnum<ChecksumType>(
          checksum_type_string_map,
          *reinterpret_cast<const ChecksumType*>(opt_address), value);
    case OptionType::kBlockBasedTableIndexType:
      return SerializeEnum<BlockBasedTableOptions::IndexType>(
          block_base_table_index_type_string_map,
          *reinterpret_cast<const BlockBasedTableOptions::IndexType*>(
              opt_address),
          value);
    case OptionType::kBlockBasedTableDataBlockIndexType:
      return SerializeEnum<BlockBasedTableOptions::DataBlockIndexType>(
          block_base_table_data_block_index_type_string_map,
          *reinterpret_cast<const BlockBasedTableOptions::DataBlockIndexType*>(
              opt_address),
          value);
    case OptionType::kBlockBasedTableIndexShorteningMode:
      return SerializeEnum<BlockBasedTableOptions::IndexShorteningMode>(
          block_base_table_index_shortening_mode_string_map,
          *reinterpret_cast<const BlockBasedTableOptions::IndexShorteningMode*>(
              opt_address),
          value);
    case OptionType::kFlushBlockPolicyFactory: {
      const auto* ptr =
          reinterpret_cast<const std::shared_ptr<FlushBlockPolicyFactory>*>(
              opt_address);
      *value = ptr->get() ? ptr->get()->Name() : kNullptrString;
      break;
    }
    case OptionType::kEncodingType:
      return SerializeEnum<EncodingType>(
          encoding_type_string_map,
          *reinterpret_cast<const EncodingType*>(opt_address), value);
    case OptionType::kWALRecoveryMode:
      return SerializeEnum<WALRecoveryMode>(
          wal_recovery_mode_string_map,
          *reinterpret_cast<const WALRecoveryMode*>(opt_address), value);
    case OptionType::kAccessHint:
      return SerializeEnum<DBOptions::AccessHint>(
          access_hint_string_map,
          *reinterpret_cast<const DBOptions::AccessHint*>(opt_address), value);
    case OptionType::kInfoLogLevel:
      return SerializeEnum<InfoLogLevel>(
          info_log_level_string_map,
          *reinterpret_cast<const InfoLogLevel*>(opt_address), value);
    case OptionType::kCompactionOptionsFIFO:
      return SerializeStruct(opt_address, value,
                             fifo_compaction_options_type_info);
    case OptionType::kCompactionOptionsUniversal:
      return SerializeStruct(opt_address, value,
                             universal_compaction_options_type_info);
    case OptionType::kCompactionStopStyle:
      return SerializeEnum<CompactionStopStyle>(
          compaction_stop_style_string_map,
          *reinterpret_cast<const CompactionStopStyle*>(opt_address), value);
    default:
      return false;
  }
  return true;
}

Status GetMutableOptionsFromStrings(
    const MutableCFOptions& base_options,
    const std::unordered_map<std::string, std::string>& options_map,
    Logger* info_log, MutableCFOptions* new_options) {
  assert(new_options);
  *new_options = base_options;
  ConfigOptions config_options;
  for (const auto& o : options_map) {
    auto iter = cf_options_type_info.find(o.first);
    if (iter == cf_options_type_info.end()) {
      return Status::InvalidArgument("Unrecognized option: " + o.first);
    }
    const auto& opt_info = iter->second;
    if (!opt_info.IsMutable()) {
      return Status::InvalidArgument("Option not changeable: " + o.first);
    }
    if (opt_info.IsDeprecated()) {
      // log warning when user tries to set a deprecated option but don't fail
      // the call for compatibility.
      ROCKS_LOG_WARN(info_log, "%s is a deprecated option and cannot be set",
                     o.first.c_str());
      continue;
    }
    Status s = opt_info.ParseOption(
        config_options, o.first, o.second,
        reinterpret_cast<char*>(new_options) + opt_info.mutable_offset);
    if (!s.ok()) {
      return s;
    }
  }
  return Status::OK();
}

Status GetMutableDBOptionsFromStrings(
    const MutableDBOptions& base_options,
    const std::unordered_map<std::string, std::string>& options_map,
    MutableDBOptions* new_options) {
  assert(new_options);
  *new_options = base_options;
  ConfigOptions config_options;

  for (const auto& o : options_map) {
    try {
      auto iter = db_options_type_info.find(o.first);
      if (iter == db_options_type_info.end()) {
        return Status::InvalidArgument("Unrecognized option: " + o.first);
      }
      const auto& opt_info = iter->second;
      if (!opt_info.IsMutable()) {
        return Status::InvalidArgument("Option not changeable: " + o.first);
      }
      Status s = opt_info.ParseOption(
          config_options, o.first, o.second,
          reinterpret_cast<char*>(new_options) + opt_info.mutable_offset);
      if (!s.ok()) {
        return s;
      }
    } catch (std::exception& e) {
      return Status::InvalidArgument("Error parsing " + o.first + ":" +
                                     std::string(e.what()));
    }
  }
  return Status::OK();
}

Status StringToMap(const std::string& opts_str,
                   std::unordered_map<std::string, std::string>* opts_map) {
  assert(opts_map);
  // Example:
  //   opts_str = "write_buffer_size=1024;max_write_buffer_number=2;"
  //              "nested_opt={opt1=1;opt2=2};max_bytes_for_level_base=100"
  size_t pos = 0;
  std::string opts = trim(opts_str);
  while (pos < opts.size()) {
    size_t eq_pos = opts.find('=', pos);
    if (eq_pos == std::string::npos) {
      return Status::InvalidArgument("Mismatched key value pair, '=' expected");
    }
    std::string key = trim(opts.substr(pos, eq_pos - pos));
    if (key.empty()) {
      return Status::InvalidArgument("Empty key found");
    }

    // skip space after '=' and look for '{' for possible nested options
    pos = eq_pos + 1;
    while (pos < opts.size() && isspace(opts[pos])) {
      ++pos;
    }
    // Empty value at the end
    if (pos >= opts.size()) {
      (*opts_map)[key] = "";
      break;
    }
    if (opts[pos] == '{') {
      int count = 1;
      size_t brace_pos = pos + 1;
      while (brace_pos < opts.size()) {
        if (opts[brace_pos] == '{') {
          ++count;
        } else if (opts[brace_pos] == '}') {
          --count;
          if (count == 0) {
            break;
          }
        }
        ++brace_pos;
      }
      // found the matching closing brace
      if (count == 0) {
        (*opts_map)[key] = trim(opts.substr(pos + 1, brace_pos - pos - 1));
        // skip all whitespace and move to the next ';'
        // brace_pos points to the next position after the matching '}'
        pos = brace_pos + 1;
        while (pos < opts.size() && isspace(opts[pos])) {
          ++pos;
        }
        if (pos < opts.size() && opts[pos] != ';') {
          return Status::InvalidArgument(
              "Unexpected chars after nested options");
        }
        ++pos;
      } else {
        return Status::InvalidArgument(
            "Mismatched curly braces for nested options");
      }
    } else {
      size_t sc_pos = opts.find(';', pos);
      if (sc_pos == std::string::npos) {
        (*opts_map)[key] = trim(opts.substr(pos));
        // It either ends with a trailing semi-colon or the last key-value pair
        break;
      } else {
        (*opts_map)[key] = trim(opts.substr(pos, sc_pos - pos));
      }
      pos = sc_pos + 1;
    }
  }

  return Status::OK();
}

Status GetStringFromStruct(
    const ConfigOptions& config_options, const void* const opt_ptr,
    const std::unordered_map<std::string, OptionTypeInfo>& type_info,
    std::string* opt_string) {
  assert(opt_string);
  opt_string->clear();
  for (const auto iter : type_info) {
    const auto& opt_info = iter.second;
    // If the option is no longer used in rocksdb and marked as deprecated,
    // we skip it in the serialization.
    if (opt_info.ShouldSerialize()) {
      const char* opt_addr =
          reinterpret_cast<const char*>(opt_ptr) + opt_info.offset;
      std::string value;
      Status s = opt_info.SerializeOption(config_options, iter.first, opt_addr,
                                          &value);
      if (s.ok()) {
        opt_string->append(iter.first + "=" + value + config_options.delimiter);
      } else {
        return s;
      }
    }
  }
  return Status::OK();
}

Status GetStringFromDBOptions(std::string* opt_string,
                              const DBOptions& db_options,
                              const std::string& delimiter) {
  ConfigOptions config_options;
  config_options.delimiter = delimiter;
  return GetStringFromDBOptions(config_options, db_options, opt_string);
}

Status GetStringFromDBOptions(const ConfigOptions& cfg_options,
                              const DBOptions& db_options,
                              std::string* opt_string) {
  return GetStringFromStruct(cfg_options, &db_options, db_options_type_info,
                             opt_string);
}

Status GetStringFromColumnFamilyOptions(std::string* opt_string,
                                        const ColumnFamilyOptions& cf_options,
                                        const std::string& delimiter) {
  ConfigOptions config_options;
  config_options.delimiter = delimiter;
  return GetStringFromColumnFamilyOptions(config_options, cf_options,
                                          opt_string);
}

Status GetStringFromColumnFamilyOptions(const ConfigOptions& config_options,
                                        const ColumnFamilyOptions& cf_options,
                                        std::string* opt_string) {
  return GetStringFromStruct(config_options, &cf_options, cf_options_type_info,
                             opt_string);
}

Status GetStringFromCompressionType(std::string* compression_str,
                                    CompressionType compression_type) {
  bool ok = SerializeEnum<CompressionType>(compression_type_string_map,
                                           compression_type, compression_str);
  if (ok) {
    return Status::OK();
  } else {
    return Status::InvalidArgument("Invalid compression types");
  }
}

static Status ParseDBOption(const ConfigOptions& config_options,
                            const std::string& name,
                            const std::string& org_value,
                            DBOptions* new_options) {
  const std::string& value = config_options.input_strings_escaped
                                 ? UnescapeOptionString(org_value)
                                 : org_value;
  auto iter = db_options_type_info.find(name);
  if (iter == db_options_type_info.end()) {
    return Status::InvalidArgument("Unrecognized option DBOptions:", name);
  } else {
    return iter->second.ParseOption(
        config_options, name, value,
        reinterpret_cast<char*>(new_options) + iter->second.offset);
  }
}

Status GetColumnFamilyOptionsFromMap(
    const ColumnFamilyOptions& base_options,
    const std::unordered_map<std::string, std::string>& opts_map,
    ColumnFamilyOptions* new_options, bool input_strings_escaped,
    bool ignore_unknown_options) {
  ConfigOptions config_options;
  config_options.ignore_unknown_options = ignore_unknown_options;
  config_options.input_strings_escaped = input_strings_escaped;
  return GetColumnFamilyOptionsFromMap(config_options, base_options, opts_map,
                                       new_options);
}

Status GetColumnFamilyOptionsFromMap(
    const ConfigOptions& config_options,
    const ColumnFamilyOptions& base_options,
    const std::unordered_map<std::string, std::string>& opts_map,
    ColumnFamilyOptions* new_options) {
  assert(new_options);
  *new_options = base_options;
  for (const auto& o : opts_map) {
    auto s =
        ParseColumnFamilyOption(config_options, o.first, o.second, new_options);
    if (!s.ok()) {
      if (s.IsNotSupported()) {
        continue;
      } else if (s.IsInvalidArgument() &&
                 config_options.ignore_unknown_options) {
        continue;
      } else {
        // Restore "new_options" to the default "base_options".
        *new_options = base_options;
        return s;
      }
    }
  }
  return Status::OK();
}

Status GetColumnFamilyOptionsFromString(
    const ColumnFamilyOptions& base_options,
    const std::string& opts_str,
    ColumnFamilyOptions* new_options) {
  ConfigOptions config_options;
  config_options.input_strings_escaped = false;
  config_options.ignore_unknown_options = false;
  return GetColumnFamilyOptionsFromString(config_options, base_options,
                                          opts_str, new_options);
}

Status GetColumnFamilyOptionsFromString(const ConfigOptions& config_options,
                                        const ColumnFamilyOptions& base_options,
                                        const std::string& opts_str,
                                        ColumnFamilyOptions* new_options) {
  std::unordered_map<std::string, std::string> opts_map;
  Status s = StringToMap(opts_str, &opts_map);
  if (!s.ok()) {
    *new_options = base_options;
    return s;
  }
  return GetColumnFamilyOptionsFromMap(config_options, base_options, opts_map,
                                       new_options);
}

Status GetDBOptionsFromMap(
    const DBOptions& base_options,
    const std::unordered_map<std::string, std::string>& opts_map,
    DBOptions* new_options, bool input_strings_escaped,
    bool ignore_unknown_options) {
  ConfigOptions config_options;
  config_options.input_strings_escaped = input_strings_escaped;
  config_options.ignore_unknown_options = ignore_unknown_options;
  return GetDBOptionsFromMap(config_options, base_options, opts_map,
                             new_options);
}

Status GetDBOptionsFromMap(
    const ConfigOptions& config_options, const DBOptions& base_options,
    const std::unordered_map<std::string, std::string>& opts_map,
    DBOptions* new_options) {
  return GetDBOptionsFromMapInternal(config_options, base_options, opts_map,
                                     new_options, nullptr);
}

Status GetDBOptionsFromMapInternal(
    const ConfigOptions& config_options, const DBOptions& base_options,
    const std::unordered_map<std::string, std::string>& opts_map,
    DBOptions* new_options,
    std::vector<std::string>* unsupported_options_names) {
  assert(new_options);
  *new_options = base_options;
  if (unsupported_options_names) {
    unsupported_options_names->clear();
  }
  for (const auto& o : opts_map) {
    auto s = ParseDBOption(config_options, o.first, o.second, new_options);
    if (!s.ok()) {
      if (s.IsNotSupported()) {
        // If the deserialization of the specified option is not supported
        // and an output vector of unsupported_options is provided, then
        // we log the name of the unsupported option and proceed.
        if (unsupported_options_names != nullptr) {
          unsupported_options_names->push_back(o.first);
        }
        // Note that we still return Status::OK in such case to maintain
        // the backward compatibility in the old public API defined in
        // rocksdb/convenience.h
      } else if (s.IsInvalidArgument() &&
                 config_options.ignore_unknown_options) {
        continue;
      } else {
        // Restore "new_options" to the default "base_options".
        *new_options = base_options;
        return s;
      }
    }
  }
  return Status::OK();
}

Status GetDBOptionsFromString(const DBOptions& base_options,
                              const std::string& opts_str,
                              DBOptions* new_options) {
  ConfigOptions config_options;
  config_options.input_strings_escaped = false;
  config_options.ignore_unknown_options = false;

  return GetDBOptionsFromString(config_options, base_options, opts_str,
                                new_options);
}

Status GetDBOptionsFromString(const ConfigOptions& config_options,
                              const DBOptions& base_options,
                              const std::string& opts_str,
                              DBOptions* new_options) {
  std::unordered_map<std::string, std::string> opts_map;
  Status s = StringToMap(opts_str, &opts_map);
  if (!s.ok()) {
    *new_options = base_options;
    return s;
  }
  return GetDBOptionsFromMap(config_options, base_options, opts_map,
                             new_options);
}

Status GetOptionsFromString(const Options& base_options,
                            const std::string& opts_str, Options* new_options) {
  ConfigOptions config_options;
  config_options.input_strings_escaped = false;
  config_options.ignore_unknown_options = false;

  return GetOptionsFromString(config_options, base_options, opts_str,
                              new_options);
}

Status GetOptionsFromString(const ConfigOptions& config_options,
                            const Options& base_options,
                            const std::string& opts_str, Options* new_options) {
  std::unordered_map<std::string, std::string> opts_map;
  Status s = StringToMap(opts_str, &opts_map);
  if (!s.ok()) {
    return s;
  }
  DBOptions new_db_options(base_options);
  ColumnFamilyOptions new_cf_options(base_options);
  for (const auto& o : opts_map) {
    if (ParseDBOption(config_options, o.first, o.second, &new_db_options)
            .ok()) {
    } else if (ParseColumnFamilyOption(config_options, o.first, o.second,
                                       &new_cf_options)
                   .ok()) {
    } else {
      return Status::InvalidArgument("Can't parse option " + o.first);
    }
  }
  *new_options = Options(new_db_options, new_cf_options);
  return Status::OK();
}

Status GetTableFactoryFromMap(
    const std::string& factory_name,
    const std::unordered_map<std::string, std::string>& opt_map,
    std::shared_ptr<TableFactory>* table_factory, bool ignore_unknown_options) {
  ConfigOptions
      config_options;  // Use default for escaped(true) and check (exact)
  config_options.ignore_unknown_options = ignore_unknown_options;
  return GetTableFactoryFromMap(config_options, factory_name, opt_map,
                                table_factory);
}

Status GetTableFactoryFromMap(
    const ConfigOptions& config_options, const std::string& factory_name,
    const std::unordered_map<std::string, std::string>& opt_map,
    std::shared_ptr<TableFactory>* table_factory) {
  Status s;
  if (factory_name == BlockBasedTableFactory().Name()) {
    BlockBasedTableOptions bbt_opt;
    s = GetBlockBasedTableOptionsFromMap(
        config_options, BlockBasedTableOptions(), opt_map, &bbt_opt);
    if (!s.ok()) {
      return s;
    }
    table_factory->reset(new BlockBasedTableFactory(bbt_opt));
    return Status::OK();
  } else if (factory_name == PlainTableFactory().Name()) {
    PlainTableOptions pt_opt;
    s = GetPlainTableOptionsFromMap(config_options, PlainTableOptions(),
                                    opt_map, &pt_opt);
    if (!s.ok()) {
      return s;
    }
    table_factory->reset(new PlainTableFactory(pt_opt));
    return Status::OK();
  }
  // Return OK for not supported table factories as TableFactory
  // Deserialization is optional.
  table_factory->reset();
  return Status::OK();
}

std::unordered_map<std::string, BlockBasedTableOptions::IndexType>
    OptionsHelper::block_base_table_index_type_string_map = {
        {"kBinarySearch", BlockBasedTableOptions::IndexType::kBinarySearch},
        {"kHashSearch", BlockBasedTableOptions::IndexType::kHashSearch},
        {"kTwoLevelIndexSearch",
         BlockBasedTableOptions::IndexType::kTwoLevelIndexSearch},
        {"kBinarySearchWithFirstKey",
         BlockBasedTableOptions::IndexType::kBinarySearchWithFirstKey}};

std::unordered_map<std::string, BlockBasedTableOptions::DataBlockIndexType>
    OptionsHelper::block_base_table_data_block_index_type_string_map = {
        {"kDataBlockBinarySearch",
         BlockBasedTableOptions::DataBlockIndexType::kDataBlockBinarySearch},
        {"kDataBlockBinaryAndHash",
         BlockBasedTableOptions::DataBlockIndexType::kDataBlockBinaryAndHash}};

std::unordered_map<std::string, BlockBasedTableOptions::IndexShorteningMode>
    OptionsHelper::block_base_table_index_shortening_mode_string_map = {
        {"kNoShortening",
         BlockBasedTableOptions::IndexShorteningMode::kNoShortening},
        {"kShortenSeparators",
         BlockBasedTableOptions::IndexShorteningMode::kShortenSeparators},
        {"kShortenSeparatorsAndSuccessor",
         BlockBasedTableOptions::IndexShorteningMode::
             kShortenSeparatorsAndSuccessor}};

std::unordered_map<std::string, EncodingType>
    OptionsHelper::encoding_type_string_map = {{"kPlain", kPlain},
                                               {"kPrefix", kPrefix}};

std::unordered_map<std::string, CompactionStyle>
    OptionsHelper::compaction_style_string_map = {
        {"kCompactionStyleLevel", kCompactionStyleLevel},
        {"kCompactionStyleUniversal", kCompactionStyleUniversal},
        {"kCompactionStyleFIFO", kCompactionStyleFIFO},
        {"kCompactionStyleNone", kCompactionStyleNone}};

std::unordered_map<std::string, CompactionPri>
    OptionsHelper::compaction_pri_string_map = {
        {"kByCompensatedSize", kByCompensatedSize},
        {"kOldestLargestSeqFirst", kOldestLargestSeqFirst},
        {"kOldestSmallestSeqFirst", kOldestSmallestSeqFirst},
        {"kMinOverlappingRatio", kMinOverlappingRatio}};

std::unordered_map<std::string, WALRecoveryMode>
    OptionsHelper::wal_recovery_mode_string_map = {
        {"kTolerateCorruptedTailRecords",
         WALRecoveryMode::kTolerateCorruptedTailRecords},
        {"kAbsoluteConsistency", WALRecoveryMode::kAbsoluteConsistency},
        {"kPointInTimeRecovery", WALRecoveryMode::kPointInTimeRecovery},
        {"kSkipAnyCorruptedRecords",
         WALRecoveryMode::kSkipAnyCorruptedRecords}};

std::unordered_map<std::string, DBOptions::AccessHint>
    OptionsHelper::access_hint_string_map = {
        {"NONE", DBOptions::AccessHint::NONE},
        {"NORMAL", DBOptions::AccessHint::NORMAL},
        {"SEQUENTIAL", DBOptions::AccessHint::SEQUENTIAL},
        {"WILLNEED", DBOptions::AccessHint::WILLNEED}};

std::unordered_map<std::string, InfoLogLevel>
    OptionsHelper::info_log_level_string_map = {
        {"DEBUG_LEVEL", InfoLogLevel::DEBUG_LEVEL},
        {"INFO_LEVEL", InfoLogLevel::INFO_LEVEL},
        {"WARN_LEVEL", InfoLogLevel::WARN_LEVEL},
        {"ERROR_LEVEL", InfoLogLevel::ERROR_LEVEL},
        {"FATAL_LEVEL", InfoLogLevel::FATAL_LEVEL},
        {"HEADER_LEVEL", InfoLogLevel::HEADER_LEVEL}};

LRUCacheOptions OptionsHelper::dummy_lru_cache_options;
CompactionOptionsUniversal OptionsHelper::dummy_comp_options_universal;
CompactionOptionsFIFO OptionsHelper::dummy_comp_options;

template <typename T1>
int offset_of(T1 LRUCacheOptions::*member) {
  return int(size_t(&(OptionsHelper::dummy_lru_cache_options.*member)) -
             size_t(&OptionsHelper::dummy_lru_cache_options));
}

template <typename T1>
int offset_of(T1 CompactionOptionsFIFO::*member) {
  return int(size_t(&(OptionsHelper::dummy_comp_options.*member)) -
             size_t(&OptionsHelper::dummy_comp_options));
}
template <typename T1>
int offset_of(T1 CompactionOptionsUniversal::*member) {
  return int(size_t(&(OptionsHelper::dummy_comp_options_universal.*member)) -
             size_t(&OptionsHelper::dummy_comp_options_universal));
}

std::unordered_map<std::string, OptionTypeInfo>
    OptionsHelper::fifo_compaction_options_type_info = {
        {"max_table_files_size",
         {offset_of(&CompactionOptionsFIFO::max_table_files_size),
          OptionType::kUInt64T, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(struct CompactionOptionsFIFO, max_table_files_size)}},
        {"ttl",
         {0, OptionType::kUInt64T, OptionVerificationType::kDeprecated,
          OptionTypeFlags::kNone, 0}},
        {"allow_compaction",
         {offset_of(&CompactionOptionsFIFO::allow_compaction),
          OptionType::kBoolean, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(struct CompactionOptionsFIFO, allow_compaction)}}};

std::unordered_map<std::string, OptionTypeInfo>
    OptionsHelper::universal_compaction_options_type_info = {
        {"size_ratio",
         {offset_of(&CompactionOptionsUniversal::size_ratio), OptionType::kUInt,
          OptionVerificationType::kNormal, OptionTypeFlags::kMutable,
          offsetof(class CompactionOptionsUniversal, size_ratio)}},
        {"min_merge_width",
         {offset_of(&CompactionOptionsUniversal::min_merge_width),
          OptionType::kUInt, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(class CompactionOptionsUniversal, min_merge_width)}},
        {"max_merge_width",
         {offset_of(&CompactionOptionsUniversal::max_merge_width),
          OptionType::kUInt, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(class CompactionOptionsUniversal, max_merge_width)}},
        {"max_size_amplification_percent",
         {offset_of(
              &CompactionOptionsUniversal::max_size_amplification_percent),
          OptionType::kUInt, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(class CompactionOptionsUniversal,
                   max_size_amplification_percent)}},
        {"compression_size_percent",
         {offset_of(&CompactionOptionsUniversal::compression_size_percent),
          OptionType::kInt, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(class CompactionOptionsUniversal,
                   compression_size_percent)}},
        {"stop_style",
         {offset_of(&CompactionOptionsUniversal::stop_style),
          OptionType::kCompactionStopStyle, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(class CompactionOptionsUniversal, stop_style)}},
        {"allow_trivial_move",
         {offset_of(&CompactionOptionsUniversal::allow_trivial_move),
          OptionType::kBoolean, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(class CompactionOptionsUniversal, allow_trivial_move)}}};

std::unordered_map<std::string, CompactionStopStyle>
    OptionsHelper::compaction_stop_style_string_map = {
        {"kCompactionStopStyleSimilarSize", kCompactionStopStyleSimilarSize},
        {"kCompactionStopStyleTotalSize", kCompactionStopStyleTotalSize}};

std::unordered_map<std::string, OptionTypeInfo>
    OptionsHelper::lru_cache_options_type_info = {
        {"capacity",
         {offset_of(&LRUCacheOptions::capacity), OptionType::kSizeT,
          OptionVerificationType::kNormal, OptionTypeFlags::kMutable,
          offsetof(struct LRUCacheOptions, capacity)}},
        {"num_shard_bits",
         {offset_of(&LRUCacheOptions::num_shard_bits), OptionType::kInt,
          OptionVerificationType::kNormal, OptionTypeFlags::kMutable,
          offsetof(struct LRUCacheOptions, num_shard_bits)}},
        {"strict_capacity_limit",
         {offset_of(&LRUCacheOptions::strict_capacity_limit),
          OptionType::kBoolean, OptionVerificationType::kNormal,
          OptionTypeFlags::kMutable,
          offsetof(struct LRUCacheOptions, strict_capacity_limit)}},
        {"high_pri_pool_ratio",
         {offset_of(&LRUCacheOptions::high_pri_pool_ratio), OptionType::kDouble,
          OptionVerificationType::kNormal, OptionTypeFlags::kMutable,
          offsetof(struct LRUCacheOptions, high_pri_pool_ratio)}}};

Status OptionTypeInfo::ParseOption(const ConfigOptions& config_options,
                                   const std::string& opt_name,
                                   const std::string& opt_value,
                                   char* opt_addr) const {
  if (IsDeprecated()) {
    return Status::OK();
  }
  try {
    if (opt_addr == nullptr) {
      return Status::NotFound("Could not find option: ", opt_name);
    } else if (parser_func != nullptr) {
      return parser_func(config_options, opt_name, opt_value, opt_addr);
    } else if (ParseOptionHelper(opt_addr, type, opt_value)) {
      return Status::OK();
    } else if (IsByName()) {
      return Status::NotSupported("Deserializing the option " + opt_name +
                                  " is not supported");
    } else {
      return Status::InvalidArgument("Error parsing:", opt_name);
    }
  } catch (std::exception& e) {
    return Status::InvalidArgument("Error parsing " + opt_name + ":" +
                                   std::string(e.what()));
  }
}

Status OptionTypeInfo::SerializeOption(const ConfigOptions& config_options,
                                       const std::string& opt_name,
                                       const char* opt_addr,
                                       std::string* opt_value) const {
  // If the option is no longer used in rocksdb and marked as deprecated,
  // we skip it in the serialization.
  Status s;
  if (opt_addr == nullptr || IsDeprecated()) {
    return Status::OK();
  } else if (string_func != nullptr) {
    return string_func(config_options, opt_name, opt_addr, opt_value);
  } else if (SerializeSingleOptionHelper(opt_addr, type, opt_value)) {
    s = Status::OK();
  } else {
    s = Status::InvalidArgument("Cannot serialize option: ", opt_name);
  }
  return s;
}

template <typename T>
bool IsOptionEqual(const char* offset1, const char* offset2) {
  return (*reinterpret_cast<const T*>(offset1) ==
          *reinterpret_cast<const T*>(offset2));
}

static bool AreEqualDoubles(const double a, const double b) {
  return (fabs(a - b) < 0.00001);
}

static bool AreOptionsEqual(OptionType type, const char* this_offset,
                            const char* that_offset) {
  switch (type) {
    case OptionType::kBoolean:
      return IsOptionEqual<bool>(this_offset, that_offset);
    case OptionType::kInt:
      return IsOptionEqual<int>(this_offset, that_offset);
    case OptionType::kUInt:
      return IsOptionEqual<unsigned int>(this_offset, that_offset);
    case OptionType::kInt32T:
      return IsOptionEqual<int32_t>(this_offset, that_offset);
    case OptionType::kInt64T: {
      int64_t v1, v2;
      GetUnaligned(reinterpret_cast<const int64_t*>(this_offset), &v1);
      GetUnaligned(reinterpret_cast<const int64_t*>(that_offset), &v2);
      return (v1 == v2);
    }
    case OptionType::kVectorInt:
      return IsOptionEqual<std::vector<int> >(this_offset, that_offset);
    case OptionType::kUInt32T:
      return IsOptionEqual<uint32_t>(this_offset, that_offset);
    case OptionType::kUInt64T: {
      uint64_t v1, v2;
      GetUnaligned(reinterpret_cast<const uint64_t*>(this_offset), &v1);
      GetUnaligned(reinterpret_cast<const uint64_t*>(that_offset), &v2);
      return (v1 == v2);
    }
    case OptionType::kSizeT: {
      size_t v1, v2;
      GetUnaligned(reinterpret_cast<const size_t*>(this_offset), &v1);
      GetUnaligned(reinterpret_cast<const size_t*>(that_offset), &v2);
      return (v1 == v2);
    }
    case OptionType::kString:
      return IsOptionEqual<std::string>(this_offset, that_offset);
    case OptionType::kDouble:
      return AreEqualDoubles(*reinterpret_cast<const double*>(this_offset),
                             *reinterpret_cast<const double*>(that_offset));
    case OptionType::kVectorCompressionType:
      return IsOptionEqual<std::vector<CompressionType> >(this_offset,
                                                          that_offset);
    case OptionType::kCompactionStyle:
      return IsOptionEqual<CompactionStyle>(this_offset, that_offset);
    case OptionType::kCompactionStopStyle:
      return IsOptionEqual<CompactionStopStyle>(this_offset, that_offset);
    case OptionType::kCompactionPri:
      return IsOptionEqual<CompactionPri>(this_offset, that_offset);
    case OptionType::kCompressionType:
      return IsOptionEqual<CompressionType>(this_offset, that_offset);
    case OptionType::kChecksumType:
      return IsOptionEqual<ChecksumType>(this_offset, that_offset);
    case OptionType::kEncodingType:
      return IsOptionEqual<EncodingType>(this_offset, that_offset);
    case OptionType::kBlockBasedTableIndexType:
      return IsOptionEqual<BlockBasedTableOptions::IndexType>(this_offset,
                                                              that_offset);
    case OptionType::kBlockBasedTableDataBlockIndexType:
      return IsOptionEqual<BlockBasedTableOptions::DataBlockIndexType>(
          this_offset, that_offset);
    case OptionType::kBlockBasedTableIndexShorteningMode:
      return IsOptionEqual<BlockBasedTableOptions::IndexShorteningMode>(
          this_offset, that_offset);
    case OptionType::kWALRecoveryMode:
      return IsOptionEqual<WALRecoveryMode>(this_offset, that_offset);
    case OptionType::kAccessHint:
      return IsOptionEqual<DBOptions::AccessHint>(this_offset, that_offset);
    case OptionType::kInfoLogLevel:
      return IsOptionEqual<InfoLogLevel>(this_offset, that_offset);
    case OptionType::kCompactionOptionsFIFO: {
      CompactionOptionsFIFO lhs =
          *reinterpret_cast<const CompactionOptionsFIFO*>(this_offset);
      CompactionOptionsFIFO rhs =
          *reinterpret_cast<const CompactionOptionsFIFO*>(that_offset);
      if (lhs.max_table_files_size == rhs.max_table_files_size &&
          lhs.allow_compaction == rhs.allow_compaction) {
        return true;
      }
      return false;
    }
    case OptionType::kCompactionOptionsUniversal: {
      CompactionOptionsUniversal lhs =
          *reinterpret_cast<const CompactionOptionsUniversal*>(this_offset);
      CompactionOptionsUniversal rhs =
          *reinterpret_cast<const CompactionOptionsUniversal*>(that_offset);
      if (lhs.size_ratio == rhs.size_ratio &&
          lhs.min_merge_width == rhs.min_merge_width &&
          lhs.max_merge_width == rhs.max_merge_width &&
          lhs.max_size_amplification_percent ==
              rhs.max_size_amplification_percent &&
          lhs.compression_size_percent == rhs.compression_size_percent &&
          lhs.stop_style == rhs.stop_style &&
          lhs.allow_trivial_move == rhs.allow_trivial_move) {
        return true;
      }
      return false;
    }
    default:
      return false;
  }  // End switch
}

bool OptionTypeInfo::MatchesOption(const ConfigOptions& config_options,
                                   const std::string& opt_name,
                                   const char* this_addr, const char* that_addr,

                                   std::string* mismatch) const {
  if (!config_options.IsCheckEnabled(GetSanityLevel())) {
    return true;  // If the sanity level is not being checked, skip it
  }
  if (this_addr == nullptr || that_addr == nullptr) {
    if (this_addr == that_addr) {
      return true;
    }
  } else if (equals_func != nullptr) {
    if (equals_func(config_options, opt_name, this_addr, that_addr, mismatch)) {
      return true;
    }
  } else if (AreOptionsEqual(type, this_addr, that_addr)) {
    return true;
  }
  if (mismatch->empty()) {
    *mismatch = opt_name;
  }
  return false;
}

bool OptionTypeInfo::MatchesByName(const ConfigOptions& config_options,
                                   const std::string& opt_name,
                                   const char* this_addr,
                                   const char* that_addr) const {
  if (IsByName()) {
    std::string that_value;
    if (SerializeOption(config_options, opt_name, that_addr, &that_value)
            .ok()) {
      return MatchesByName(config_options, opt_name, this_addr, that_value);
    }
  }
  return false;
}

bool OptionTypeInfo::MatchesByName(const ConfigOptions& config_options,
                                   const std::string& opt_name,
                                   const char* opt_addr,
                                   const std::string& that_value) const {
  std::string this_value;
  if (!IsByName()) {
    return false;
  } else if (!SerializeOption(config_options, opt_name, opt_addr, &this_value)
                  .ok()) {
    return false;
  } else if (IsEnabled(OptionVerificationType::kByNameAllowFromNull)) {
    if (that_value == kNullptrString) {
      return true;
    }
  } else if (IsEnabled(OptionVerificationType::kByNameAllowNull)) {
    if (that_value == kNullptrString) {
      return true;
    }
  }
  return (this_value == that_value);
}
#endif  // !ROCKSDB_LITE

}  // namespace ROCKSDB_NAMESPACE
