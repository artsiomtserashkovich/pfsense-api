<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Auth;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for packages that are available to install on this system.
 */
class AvailablePackage extends Model
{
    const PACKAGES_CACHE_FILE = "/usr/local/share/pfSense-pkg-RESTAPI/packages.json";
    const PACKAGES_CACHE_EXPIRATION_SECONDS = 60 * 60;    // Cache expires hourly

    public StringField $name;
    public StringField $shortname;
    public StringField $descr;
    public StringField $version;
    public BooleanField $installed;
    public StringField $deps;

    public function __construct(mixed $id = null,mixed $data = [], Auth $client = null, bool $async = true)
    {
        # Set model attributes
        $this->internal_callable = "get_all_pkgs";
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            help_text: "The name of the pfSense package."
        );
        $this->shortname = new StringField(
            default: "",
            read_only: true,
            help_text: "The package's description."
        );
        $this->descr = new StringField(
            default: "",
            read_only: true,
            help_text: "The package's description."
        );
        $this->version = new StringField(
            default: "",
            read_only: true,
            help_text: "The latest version available for this package."
        );
        $this->installed = new BooleanField(
            default: false,
            read_only: true,
            indicates_true: true,
            indicates_false: false,
            help_text: "Indicates whether the package is currently installed or not."
        );
        $this->deps = new StringField(
            default: [],
            read_only: true,
            many: true,
            delimiter: null,
            help_text: "Dependencies for this package that are also installed when this package is installed."
        );

        parent::__construct(id: $id, data: $data, client: $client, async: $async);
    }

    /**
     * Obtains all available packages and their associated attributes. This method is the internal callable that is
     * used to populate this Model's Field object internal values.
     * @return array An array containing the internal values of all available packages
     * @note To avoid flooding the package servers with requests, this method will automatically cache package
     * information obtained from the remote repos hourly.
     */
    protected function get_all_pkgs(): array {
        # Variables
        $pkgs = [];
        $pkgs_cache = json_decode(file_get_contents(self::PACKAGES_CACHE_FILE), true);
        $current_time = time();

        # Check if our previous packages cache has expired
        if ($pkgs_cache["last_updated"] + self::PACKAGES_CACHE_EXPIRATION_SECONDS < $current_time) {
            # Pull package data from the remote package repos
            $pkg_resp = get_pkg_info();
            $pkgs_cache = ["last_updated" => $current_time, "packages" => $pkg_resp];
            file_put_contents(self::PACKAGES_CACHE_FILE, json_encode($pkgs_cache));
        }

        # Loop through each of the output lines and format it
        foreach ($pkgs_cache["packages"] as $pkg) {
            # Add our formatted package to our array
            $pkg_data = [
                "name" => $pkg["name"],
                "shortname" => $pkg["shortname"],
                "version" => $pkg["version"],
                "descr" => $pkg["desc"],
                "installed" => is_package_installed($pkg["shortname"]),
                "deps" => []
            ];

            # If this package has dependencies, include them
            if ($pkg["deps"]) {
                foreach($pkg["deps"] as $dep => $dep_data) {
                    $pkg_data["deps"][] = $dep."-".$dep_data["version"];
                }
            }

            $pkgs[] = $pkg_data;
        }

        return $pkgs;
    }
}