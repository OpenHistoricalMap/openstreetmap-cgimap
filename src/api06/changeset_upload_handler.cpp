#include "cgimap/api06/changeset_upload_handler.hpp"
#include "cgimap/request_helpers.hpp"
#include "cgimap/http.hpp"
#include "cgimap/config.hpp"

#include "cgimap/backend/apidb/changeset_upload/changeset_updater.hpp"
#include "cgimap/infix_ostream_iterator.hpp"
#include "cgimap/backend/apidb/changeset_upload/node_updater.hpp"
#include "cgimap/api06/changeset_upload/osmchange_handler.hpp"
#include "cgimap/api06/changeset_upload/osmchange_tracking.hpp"
#include "cgimap/api06/changeset_upload/osmchange_input_format.hpp"
#include "cgimap/backend/apidb/changeset_upload/relation_updater.hpp"
#include "cgimap/backend/apidb/changeset_upload/transaction_manager.hpp"
#include "cgimap/backend/apidb/changeset_upload/way_updater.hpp"
//#include "cgimap/api06/changeset_upload/osmchange_input_format.hpp"

#include "types.hpp"
#include "util.hpp"

#include <sstream>

using std::stringstream;
using std::vector;

namespace api06 {

changeset_upload_responder::changeset_upload_responder(
  mime::type mt, osm_changeset_id_t id_, data_selection_ptr &w_)
  : osmchange_responder(mt, w_), id(id_) {

 // throw http::server_error("unimplemented");

  int changeset = id_;
  int uid = 1;

  std::string data = "";

  Transaction_Manager m { "dbname=openstreetmap" };

  std::shared_ptr<OSMChange_Tracking> change_tracking(std::make_shared<OSMChange_Tracking>());


  // TODO: we're still in api06 code, don't use ApiDB_*_Updater here!!

  std::unique_ptr<Changeset_Updater> changeset_updater(new Changeset_Updater(m, changeset, uid));
  std::unique_ptr<Node_Updater> node_updater(new ApiDB_Node_Updater(m, change_tracking));
  std::unique_ptr<Way_Updater> way_updater(new ApiDB_Way_Updater(m, change_tracking));
  std::unique_ptr<Relation_Updater> relation_updater(new ApiDB_Relation_Updater(m, change_tracking));


  changeset_updater->lock_current_changeset();

  OSMChange_Handler handler(std::move(node_updater), std::move(way_updater), std::move(relation_updater),
                  changeset, uid);

  OSMChangeXMLParser parser(&handler);

  parser.process_message(data);

  handler.finish_processing();

  changeset_updater->update_changeset(handler.get_num_changes(), handler.get_bbox());

  m.commit();

  // Result: change_tracking->get_xml_diff_result();


}

changeset_upload_responder::~changeset_upload_responder() {}

changeset_upload_handler::changeset_upload_handler(request &req, osm_changeset_id_t id_)
  : handler(mime::unspecified_type, http::method::POST | http::method::OPTIONS)
  , id(id_) {
}

changeset_upload_handler::~changeset_upload_handler() {}

std::string changeset_upload_handler::log_name() const { return "changeset/upload"; }

responder_ptr_t changeset_upload_handler::responder(data_selection_ptr &w) const {
  return responder_ptr_t(new changeset_upload_responder(mime_type, id, w));
}

} // namespace api06
