// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.

#include "ccf/app_interface.h"
#include "ccf/common_auth_policies.h"
#include "ccf/crypto/verifier.h"
#include "ccf/ds/hash.h"
#include "ccf/historical_queries_adapter.h"
#include "ccf/http_query.h"
#include "ccf/indexing/strategies/seqnos_by_key_bucketed.h"
#include "ccf/json_handler.h"
#include "ccf/version.h"

#include <charconv>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

namespace app {

using Map = kv::Map<size_t, std::string>;
static constexpr auto RECORDS = "records";

struct Record {
  struct In {
    std::string msg;
  };
  In in;

  using Out = void;
};
DECLARE_JSON_TYPE(Record::In);
DECLARE_JSON_REQUIRED_FIELDS(Record::In, msg);

DECLARE_JSON_TYPE(Record);
DECLARE_JSON_REQUIRED_FIELDS(Record, in);

class AppHandlers : public ccf::UserEndpointRegistry {
public:
  AppHandlers(ccfapp::AbstractNodeContext &context)
      : ccf::UserEndpointRegistry(context) {
    openapi_info.title = "CCF Sample C++ App";
    openapi_info.description = "This minimal CCF C++ application aims to be "
                               "used as a template for CCF developers.";
    openapi_info.document_version = "0.0.1";

    auto record = [this](auto &ctx, nlohmann::json &&params) {
      const auto parsed_query =
          http::parse_query(ctx.rpc_ctx->get_request_query());

      std::string error_reason;
      size_t id = 0;
      if (!http::get_query_value(parsed_query, "id", id, error_reason)) {
        return ccf::make_error(HTTP_STATUS_BAD_REQUEST,
                               ccf::errors::InvalidQueryParameterValue,
                               std::move(error_reason));
      }

      const auto in = params.get<Record::In>();
      if (in.msg.empty()) {
        return ccf::make_error(HTTP_STATUS_BAD_REQUEST,
                               ccf::errors::InvalidInput,
                               "Cannot record an empty log message.");
      }

      auto records_handle = ctx.tx.template rw<Map>(RECORDS);
      records_handle->put(id, in.msg);
      return ccf::make_success();
    };

    make_endpoint("/log/", HTTP_POST, ccf::json_adapter(record),
                  ccf::no_auth_required)
        .set_auto_schema<Record>()
        .add_query_parameter<size_t>("id")
        .install();
  }
};
} // namespace app

namespace ccfapp {
std::unique_ptr<ccf::endpoints::EndpointRegistry>
make_user_endpoints(ccfapp::AbstractNodeContext &context) {
  return std::make_unique<app::AppHandlers>(context);
}
} // namespace ccfapp