<?php
/**
 * @file
 * janet_soap_api.api.inc
 *
 * Contains API hooks instigated by janet_soap_api module
 */

/**
 * hook_janet_soap_api_api_key_operations()
 *
 * A function to allow other modules to perform actions on keys, as
 * 'multiple key update' functionality
 *
 * @see janet_soap_api_janet_soap_api_api_key_operations()
 */
function hook_janet_soap_api_api_key_operations() {
  $operations = array(
    'activate_keys' => array(
      'label' => t('Make Keys Active'),
      'callback' => '_janet_soap_api_update_keys',
      'callback arguments' => array('updates' => array('active' => 1)),
    ),
    'deactivate_keys' => array(
      'label' => t('Make Keys Inactive'),
      'callback' => '_janet_soap_api_update_keys',
      'callback arguments' => array('updates' => array('active' => 0)),
    ),
    'delete_keys' => array(
      'label' => t('Delete Keys'),
      'callback' => '_janet_soap_api_delete_keys',
    ),
  );

  return $operations;
}
