<?php

namespace RESTAPI\Auth;

use RESTAPI\Core\Auth;

class BasicAuth extends Auth
{
    const WWW_AUTHENTICATE_HEADER = 'WWW-Authenticate: Basic realm="Login for pfSense REST API"';

    /**
     * @var array $security_scheme The OpenAPI security scheme for this Auth class.
     */
    public array $security_scheme = ["type" => "http", "scheme" => "basic"];

    /**
     * Obtains the submitted username and password from the client using Basic authentication and performs
     * authentication against the pfSense local user database.
     * @link Basic https://developer.mozilla.org/en-US/docs/Web/HTTP/Authentication
     * @return bool Returns true if basic auth is successful, return false otherwise
     */
    public function _authenticate(): bool
    {
        # Obtain the username and password from the client via Basic authentication
        $this->username = ($_SERVER["PHP_AUTH_USER"]) ?: "";
        $password = ($_SERVER["PHP_AUTH_PW"]) ?: "";

        # Authenticate against the local user database and return the result
        return (bool)authenticate_user($this->username, $password);
    }

    /**
     * When basic authentication fails, set the WWW-Authenticate to request basic authentication.
     */
    protected function on_failed_auth(): void
    {
        header(self::WWW_AUTHENTICATE_HEADER);
    }
}