<?php

namespace API\Dispatchers;

use API\Core\Command;
use API\Core\Dispatcher;

class WebGUIRestartDispatcher extends Dispatcher
{
    # Don't limit the number of Web GUI restarts that can run at a time.
    public int $max_concurrency = 0;

    /**
     * Restarts the web GUI.
     */
    public static function process(...$arguments)
    {
        # Wait a second to prevent prematurely killing API calls
        sleep(3);
        new Command("/etc/rc.restart_webgui");
    }

}