/**
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This file is part of openstreetmap-cgimap (https://github.com/zerebubuth/openstreetmap-cgimap/).
 *
 * Copyright (C) 2009-2023 by the CGImap developer community.
 * For a full list of authors see the git log.
 */

#ifndef API06_RELATION_VERSION_HANDLER_HPP
#define API06_RELATION_VERSION_HANDLER_HPP

#include "cgimap/handler.hpp"
#include "cgimap/osm_current_responder.hpp"
#include "cgimap/request.hpp"
#include <string>

namespace api06 {

class relation_version_responder : public osm_current_responder {
public:
  relation_version_responder(mime::type, osm_nwr_id_t, osm_version_t v, data_selection &);

private:
  osm_nwr_id_t id;
  osm_version_t v;

  void check_visibility();
};

class relation_version_handler : public handler {
public:
  relation_version_handler(request &req, osm_nwr_id_t id, osm_version_t v);

  std::string log_name() const override;
  responder_ptr_t responder(data_selection &x) const override;

private:
  osm_nwr_id_t id;
  osm_version_t v;
};

} // namespace api06

#endif /* API06_RELATION_VERSION_HANDLER_HPP */
