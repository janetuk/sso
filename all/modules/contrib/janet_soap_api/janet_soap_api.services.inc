<?php
/**
 * @file
 * janet_soap_api.services.inc
 */

/**
 * Implements hook_default_services_endpoint().
 */
function janet_soap_api_default_services_endpoint() {
  $export = array();

  $endpoint = new stdClass();
  $endpoint->disabled = FALSE; /* Edit this to true to make a default endpoint disabled initially */
  $endpoint->api_version = 3;
  $endpoint->name = 'soap_api';
  $endpoint->server = 'soap_server';
  $endpoint->path = 'api/soap';
  $endpoint->authentication = array(
    'janet_soap_api' => array(
      'options' => array(
        'operation' => 'activate_keys',
        'submit' => 'Update',
      ),
      'keys' => array(),
    ),
  );
  $endpoint->server_settings = array();
  $endpoint->resources = array(
    /**
     * these should be added individually as required but the below *should* be
     * the default set for community for now. new functions should be
     * refeaturised into this function
     */
    'shib_auth_api' => array(
      'operations' => array(
        'retrieve' => array(
          'enabled' => '1',
        ),
      ),
    ),
    'soap_crm' => array(
      'operations' => array(
        'retrieve' => array(
          'enabled' => '1',
        ),
      ),
    ),
    'sso_id_api' => array(
      'operations' => array(
        'retrieve' => array(
          'enabled' => '1',
        ),
      ),
    ),
    'sso_user_api' => array(
      'operations' => array(
        'retrieve' => array(
          'enabled' => '1',
        ),
      ),
    ),
    'verify' => array(
      'operations' => array(
        'retrieve' => array(
          'enabled' => '1',
        ),
        'verify_user' => array(
          'enabled' => '1',
        ),
      ),
    ),
  );
  $endpoint->debug = 0;
  $export['soap_api'] = $endpoint;

  return $export;
}
