<?php

namespace API\Tests;

require_once("api/auto_loader.inc");

use API\Core\Command;
use API\Core\TestCase;
use API\Models\VirtualIP;

class APIModelsVirtualIPTestCase extends TestCase
{
    /**
     * Checks that the virtual IP `type` cannot be `network` when `mode` is set to
     * `ipalias` or `carp`.
     */
    public function test_type_cannot_be_network_with_ipalias_or_carp() {
        $this->assert_throws_response(
            response_id: "VIRTUALIP_MODE_DOES_NOT_SUPPORT_NETWORK_TYPE",
            code: 400,
            callable: function () {
                $test_model = new VirtualIP();
                $test_model->mode->value = "carp";
                $test_model->validate_type("network");
            }
        );

        $this->assert_throws_response(
            response_id: "VIRTUALIP_MODE_DOES_NOT_SUPPORT_NETWORK_TYPE",
            code: 400,
            callable: function () {
                $test_model = new VirtualIP();
                $test_model->mode->value = "ipalias";
                $test_model->validate_type("network");
            }
        );
    }

    /**
     * Checks that the virtual IP `subnet_bits` is effectively limited to >=32 for IPv4 vips
     */
    public function test_ipv4_subnet_bits_is_limited() {
        $this->assert_throws_response(
            response_id: "VIRTUALIP_IPV4_SUBNET_BITS_EXCEEDED",
            code: 400,
            callable: function () {
                $test_model = new VirtualIP(
                    data: [
                        "mode" => "ipalias",
                        "interface" => "wan",
                        "subnet" => "1.2.3.4",
                        "subnet_bits" => 33
                    ]
                );
                $test_model->validate();
            }
        );

        $this->assert_does_not_throw(
            callable: function () {
                $test_model = new VirtualIP(
                    data: [
                        "mode" => "ipalias",
                        "interface" => "wan",
                        "subnet" => "1.2.3.4",
                        "subnet_bits" => 32
                    ]
                );
                $test_model->validate();
            }
        );
    }

    /**
     * Checks that pending virtual IP changes are properly serialized before applying.
     */
    public function test_serialize_changes() {
        # Create a vip but don't apply it so we can check the serialized changes file
        $test_model_1 = new VirtualIP(
            data: [
                "mode" => "ipalias",
                "interface" => "wan",
                "subnet" => "1.2.3.4",
                "subnet_bits" => 24
            ]
        );
        $test_model_1->create();

        # Ensure the file exists
        $this->assert_is_true(is_file("/tmp/.firewall_virtual_ip.apply"));

        # Apply the changes and ensure the serialized changes file is no longer present
        $test_model_1->apply();
        $this->assert_is_false(is_file("/tmp/.firewall_virtual_ip.apply"));

        $test_model_1->delete();
    }

    /**
     * Checks that ipalias virtual IPs are actually configured after applying.
     */
    public function test_ipalias_virtual_ip() {
        # Create a new ipalias virtual IP and ensure it is present in ifconfig afterward
        $test_model = new VirtualIP(
            data: [
                "mode" => "ipalias",
                "interface" => "wan",
                "subnet" => "1.2.3.4",
                "subnet_bits" => 24
            ]
        );
        $test_model->create(apply: true);
        $ifconfig = new Command("ifconfig");
        $this->assert_is_true(
            str_contains($ifconfig->output, "inet 1.2.3.4 netmask 0xffffff00 broadcast 1.2.3.255")
        );

        # Update the IP alias vip and ensure the previous vip is no longer present and the new vip is present
        $test_model->subnet->value = "4.3.2.1";
        $test_model->subnet_bits->value = 25;
        $test_model->update(apply: true);
        $ifconfig = new Command("ifconfig");
        $this->assert_is_false(
            str_contains($ifconfig->output, "inet 1.2.3.4 netmask 0xffffff00 broadcast 1.2.3.255")
        );
        $this->assert_is_true(
            str_contains($ifconfig->output, "inet 4.3.2.1 netmask 0xffffff80 broadcast 4.3.2.127")
        );

        # Remove the vip and ensure it is no longer present in ifconfig
        $test_model->delete();
        $ifconfig = new Command("ifconfig");
        $this->assert_is_false(
            str_contains($ifconfig->output, "inet 4.3.2.1 netmask 0xffffff80 broadcast 4.3.2.127")
        );
    }

    /**
     * Checks that carp virtual IPs are actually configured after applying.
     */
    public function test_carp_virtual_ip() {
        # Create a new virtual IP and ensure it is present in ifconfig afterward
        $test_model = new VirtualIP(
            data: [
                "mode" => "carp",
                "interface" => "wan",
                "subnet" => "4.5.6.7",
                "subnet_bits" => 24,
                "vhid" => 7,
                "advbase" => 10,
                "advskew" => 11,
                "password" => "testpasswd"
            ]
        );
        $test_model->create(apply: true);
        $ifconfig = new Command("ifconfig");
        $this->assert_is_true(
            str_contains($ifconfig->output, "inet 4.5.6.7 netmask 0xffffff00 broadcast 4.5.6.255 vhid 7")
        );
        $this->assert_is_true(
            str_contains($ifconfig->output, "advbase 10 advskew 11")
        );

        # Update the vip and ensure the previous vip is no longer present and the new vip is present
        $test_model->subnet->value = "7.6.5.4";
        $test_model->subnet_bits->value = 25;
        $test_model->vhid->value = 5;
        $test_model->advbase->value = 5;
        $test_model->advskew->value = 6;
        $test_model->update(apply: true);
        $ifconfig = new Command("ifconfig");
        $this->assert_is_false(
            str_contains($ifconfig->output, "inet 4.5.6.7 netmask 0xffffff00 broadcast 4.5.6.255 vhid 7")
        );
        $this->assert_is_false(
            str_contains($ifconfig->output, "advbase 10 advskew 11")
        );
        $this->assert_is_true(
            str_contains($ifconfig->output, "inet 7.6.5.4 netmask 0xffffff80 broadcast 7.6.5.127 vhid 5")
        );
        $this->assert_is_true(
            str_contains($ifconfig->output, "advbase 5 advskew 6")
        );

        # Remove the vip and ensure it is no longer present in ifconfig
        $test_model->delete();
        $ifconfig = new Command("ifconfig");
        $this->assert_is_false(
            str_contains($ifconfig->output, "inet 7.6.5.4 netmask 0xffffff80 broadcast 7.6.5.127 vhid 5")
        );
    }

    /**
     * Checks that proxyarp virtual IPs are actually configured after applying.
     */
    public function test_proxyarp_virtual_ip() {
        # Create a new proxyarp virtual IP and ensure it is present in ifconfig afterward
        $test_model = new VirtualIP(
            data: [
                "mode" => "proxyarp",
                "interface" => "wan",
                "subnet" => "1.2.3.4",
                "subnet_bits" => 32
            ]
        );
        $test_model->create(apply: true);
        $choparp = new Command("/usr/bin/top -baHS 999 | grep choparp");
        $this->assert_is_true(
            str_contains($choparp->output, "1.2.3.4")
        );

        # Update the IP alias vip and ensure the previous vip is no longer present and the new vip is present
        $test_model->subnet->value = "4.3.2.1";
        $test_model->update(apply: true);
        $choparp = new Command("/usr/bin/top -baHS 999 | grep choparp");
        $this->assert_is_false(
            str_contains($choparp->output, "1.2.3.4")
        );
        $this->assert_is_true(
            str_contains($choparp->output, "4.3.2.1")
        );

        # Remove the vip and ensure it is no longer present in ifconfig
        $test_model->delete();
        $choparp = new Command("/usr/bin/top -baHS 999 | grep choparp");
        $this->assert_is_false(
            str_contains($choparp->output, "4.3.2.1")
        );
    }
}