<?php

namespace API\Endpoints;

require_once("api/auto_loader.inc");

use API\Core\Endpoint;

class StaticRoute extends Endpoint
{
    public function __construct() {
        # Assign the URL for this endpoint along with the Model it corresponds to.
        $this->url = "/api/v2/system/routing/static_route";
        $this->model_name = "StaticRoute";
        $this->tag = "ROUTING";

        # Assign allowed request methods along with the privileges required for each
        $this->request_method_options = ["GET", "POST", "PATCH", "DELETE"];
        $this->get_privileges = ["page-all", "page-system-staticroutes"];
        $this->post_privileges = ["page-all", "page-system-staticroutes-editroute"];
        $this->patch_privileges = ["page-all", "page-system-staticroutes-editroute"];
        $this->delete_privileges = ["page-all", "page-system-staticroutes-editroute"];

        # Construct the parent Endpoint object
        parent::__construct();
    }
}