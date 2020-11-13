#ifndef RTSYS_PROTOBUF_MISC_CPP_H
#define RTSYS_PROTOBUF_MISC_CPP_H

// ===
// === Include
// ============================================================================ //

#include "protobuf/misc/misc_cpp_delimited.h"
#include <google/protobuf/util/json_util.h>

// ===
// === Function
// ============================================================================ //

namespace rtsys {
namespace protobuf {
namespace misc {

/**
 * @brief This serializes the message in prefixed-size format.
 */
inline std::string serializeDelimitedToString(const google::protobuf::MessageLite &message)
{
    std::string raw_stream;
    google::protobuf::io::StringOutputStream zero_copy_stream(&raw_stream);
    google::protobuf::util::SerializeDelimitedToZeroCopyStream(message, &zero_copy_stream);
    return raw_stream;
}

/**
 * @brief This parses the prefixed-size formatted message.
 */
inline void parseDelimitedFromString(google::protobuf::MessageLite *message, const std::string &raw_stream)
{
    bool *clean_eof = nullptr;
    google::protobuf::io::ArrayInputStream zero_copy_stream(raw_stream.data(), static_cast<int>(raw_stream.length()));
    google::protobuf::util::ParseDelimitedFromZeroCopyStream(message, &zero_copy_stream, clean_eof);
}

/**
 * @brief This serializes the message in json format.
 */
inline google::protobuf::util::Status serializeToJson(const google::protobuf::Message &message,
                                                      std::string *json_stream)
{
    google::protobuf::util::JsonOptions json_options;
    json_options.add_whitespace = true;
    json_options.preserve_proto_field_names = true;
    return google::protobuf::util::MessageToJsonString(message, json_stream, json_options);
}

/**
 * @brief This parses the json formatted message.
 */
inline google::protobuf::util::Status parseFromJson(google::protobuf::Message *message, const std::string &json_stream)
{
    return google::protobuf::util::JsonStringToMessage(json_stream, message);
}

} // namespace misc
} // namespace protobuf
} // namespace rtsys

#endif // RTSYS_PROTOBUF_MISC_CPP_H
