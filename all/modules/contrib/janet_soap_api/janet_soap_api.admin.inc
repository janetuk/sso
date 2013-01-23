<?php
/**
 * @file
 * janet_soap_api.admin.inc
 * Code for admin functions on the SOAP Server Authentication module
 * by Janet API Key.
 */

/**
 * Callback page for the /admin/structure/services/list/%soap_server_endpoint/authentication
 * page
 *
 * @param $settings
 * @param $form_state
 * @return mixed
 */
function janet_soap_api_authentication_page($settings, &$form_state) {
  $destination = drupal_get_destination();

  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();

  // Create our own javascript that will be used to theme a modal.
  $sample_style = array(
    'ctools-janet-modal-style' => array(
      'modalSize' => array(
        'type' => 'fixed',
        'width' => 700,
        'height' => 500,
        'addWidth' => 5,
        'addHeight' => 5,
      ),
      'modalOptions' => array(
        'opacity' => .7,
        'background-color' => '#000',
      ),
      'animation' => 'fadeIn',
      'modalTheme' => 'CToolsJanetModal',
      'throbber' => theme('image', array('path' => ctools_image_path('ajax-loader.gif', 'janet_soap_api'), 'alt' => t('Loading...'), 'title' => t('Loading'))),
    ),
  );

  drupal_add_js($sample_style, 'setting');
  // Since we have our js, css and images in well-known named directories,
  // CTools makes it easy for us to just use them without worrying about
  // using drupal_get_path() and all that ugliness.
  ctools_add_js('janet-soap-api', 'janet_soap_api');
  ctools_add_css('janet-soap-api', 'janet_soap_api');

  $form['addlink'] = array(
    '#prefix' => '<ul class="action-links">',
    '#markup' => '<li>'.l(t('Add new Key'), current_path().'/nojs/add', array(
      'attributes' => array('class' => array('ctools-use-modal', 'ctools-modal-ctools-janet-modal-style')),
      'query' => array('destination' => $destination['destination']),
    )).'</li>',
    '#suffix' => '</ul>',
    '#type' => 'markup',
    '#attributes' => array(
    ),
  );

  // Build the 'Update options' form.
  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#attributes' => array('class' => array('container-inline')),
  );

  $options = array();
  foreach (module_invoke_all('janet_soap_api_api_key_operations') as $operation => $array) {
    $options[$operation] = $array['label'];
  }

  $form['options']['operation'] = array(
    '#type' => 'select',
    '#title' => t('Operation'),
    '#title_display' => 'invisible',
    '#options' => $options,
    '#default_value' => 'approve',
  );
  $form['options']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#submit' => array('_janet_soap_api_keys_update_submit'),
  );

  // Build the sortable table header.
  $header = array(
    'key' => array('data' => t('API Key'), 'field' => 'k.api_key', 'sort' => 'asc'),
    'description' => array('data' => t('Description'), 'field' => 'k.description'),
    'author' => array('data' => t('Created By')),
    'status' => array('data' => t('Status'), 'field' => 'k.status'),
    'changed' => array('data' => t('Updated'), 'field' => 'k.changed'),
    'operations' => array('data' => t('Operations')),
  );

  $query = db_select('janet_api_keys', 'k')
      ->extend('PagerDefault')
      ->extend('TableSort');

  $keys = $query
    ->fields('k')
    ->limit(50)
    ->orderByHeader($header)
    ->execute()
    ->fetchAll();

  // Prepare the list of keys.
  $options = array();
  foreach ($keys as $key) {
    $options[$key->kid] = array(
      'key' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => $key->api_key,
          '#href' => current_path().'/nojs/' . $key->kid . '/edit',
          '#options' => array(
            'attributes' => array(
              'class' => array('ctools-use-modal', 'ctools-modal-ctools-janet-modal-style'),
            ),
          ),
        ),
      ),
      'description' => $key->description,
      'author' => theme('username', array('account' => user_load($key->uid))),
      'status' => $key->status ? t('active') : t('not active'),
      'changed' => format_date($key->changed, 'short'),
    );

    // Build a list of all the accessible operations for the current node.
    $operations = array(
      'edit' => array(
        'title' => t('edit'),
        'href' => current_path().'/nojs/' . $key->kid . '/edit',
        'query' => $destination,
        'attributes' => array(
          'class' => array('ctools-use-modal', 'ctools-modal-ctools-janet-modal-style'),
        ),
      ),
      'delete' => array(
        'title' => t('delete'),
        'href' => current_path().'/nojs/' . $key->kid . '/delete',
        'query' => $destination,
        'attributes' => array(
          'class' => array('ctools-use-modal', 'ctools-modal-ctools-janet-modal-style'),
        ),
      )
    );
    // Render an unordered list of operations links.
    $options[$key->kid]['operations'] = array(
      'data' => array(
        '#theme' => 'links__node_operations',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline')),
      ),
    );
  }

  $form['keys'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#attributes' => array(),
    '#empty' => t('No keys created yet. Add one above.'),
  );
  $form['pager'] = array('#markup' => theme('pager'));

  return $form;
}

/**
 * Implements submit hook for the 'operations' section of the edit
 * form.
 *
 * This uses hook_janet_soap_api_api_key_operations to load available
 * functions.
 *
 * @see hook_janet_soap_api_api_key_operations
 *
 * @param $form
 * @param $form_state
 */
function _janet_soap_api_keys_update_submit($form, &$form_state) {
  $values = &$form_state['values'];

  $form_operation = $values['janet_soap_api']['options']['operation'];
  $keys = array_filter($values['janet_soap_api']['keys']);

  foreach (module_invoke_all('janet_soap_api_api_key_operations') as $operation => $action) {
    if($form_operation == $operation) {
      break;
    }
  }
  // $action now holds the action we wish to perform
  if((!empty($action['callback'])) && (function_exists($action['callback']))) {
    if(empty($action['callback arguments'])) {
      $action['callback arguments'] = array();
    }
    call_user_func_array($action['callback'], array($keys, $action['callback arguments']));
  }
}

/**
 * Edit key form page callback
 *
 * Called by CTOOLS Ajax (Modal) or nojs equivalent
 *
 * @see janet_soap_api_edit_key_form() for actual form
 *
 * @param $server_endpoint
 * @param bool $nojs
 * @param $api_key
 *   object, not string
 * @return string
 */
function janet_soap_api_edit_key($server_endpoint, $js = false, $api_key) {
  $title = t('Edit Key: !key', array('!key' => $api_key->api_key));

  $form_state = array(
    'ajax' => TRUE,
    'title' => $title,
    'server_endpoint' => $server_endpoint,
    'key' => $api_key,
  );

  if ($js) {
    ctools_include('ajax');
    ctools_include('modal');

    $output = ctools_modal_form_wrapper('janet_soap_api_edit_key_form', $form_state);

    if (!empty($form_state['executed'])) {
      $output[] = ctools_ajax_command_reload();
    }
    print ajax_render($output);
  }
  else {
    drupal_set_title($title);
    return drupal_get_form('janet_soap_api_edit_key_form', $form_state);
  }
}

/**
 * Add key form page callback
 *
 * Called by CTOOLS Ajax (Modal) or nojs equivalent
 *
 * @see janet_soap_api_edit_key_form() for actual form
 *
 * @param $server_endpoint
 * @param bool $nojs
 * @param $api_key
 *   object, not string
 * @return string
 */
function janet_soap_api_add_key($server_endpoint, $js = false) {
  $title = t('Add New API Key');

  $form_state = array(
    'ajax' => TRUE,
    'title' => $title,
    'server_endpoint' => $server_endpoint,
    'key' => (object)array('kid' => 0),
  );

  if ($js) {
    ctools_include('ajax');
    ctools_include('modal');

    $output = ctools_modal_form_wrapper('janet_soap_api_edit_key_form', $form_state);

    if (!empty($form_state['executed'])) {
      $output[] = ctools_ajax_command_reload();
    }
    print ajax_render($output);
  }
  else {
    drupal_set_title($title);
    return drupal_get_form('janet_soap_api_edit_key_form', $form_state);
  }
}

/**
 * Form callback for add and edit form
 *
 * Called from add and edit functions
 * @see janet_soap_api_edit_key()
 * @see janet_soap_api_add_key()
 *
 * @param $form
 * @param $form_state
 * @param null $extra_values
 * @return array
 */
function janet_soap_api_edit_key_form($form, &$form_state, $extra_values = null) {
  if(empty($form_state['ajax'])) {
    // called through ctools modal
    $key = $extra_values['key'];
    $endpoint = $extra_values['server_endpoint'];
  } else {
    $key = $form_state['key'];
    $endpoint = $form_state['server_endpoint'];
  }

  $form['kid'] = array(
    '#type' => 'value',
    '#value' => $key->kid,
  );
  $form['endpoint_name'] = array(
    '#type' => 'value',
    '#value' => $endpoint->name,
  );

  $form['key_details'] = array(
    '#type' => 'fieldset',
    '#title' => t('Key Details'),
  );

  $form['key_details']['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#size' => 60,
    '#required' => TRUE,
    '#disabled' => TRUE,
    '#description' => t('This is the API Key to be handed out to the client'),
    '#default_value' => (!empty($key->api_key)) ? $key->api_key : _janet_soap_api_generate_key(),
  );
  $form['key_details']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#description' => t('An overview description of the Key'),
    '#required' => TRUE,
    '#maxlength' => '255',
    '#default_value' => (!empty($key->description)) ? $key->description : '',
  );
  $form['key_details']['recipient'] = array(
    '#type' => 'textfield',
    '#title' => t('Recipient'),
    '#description' => t('The name of the person against whom this key is registered'),
    '#required' => TRUE,
    '#maxlength' => '255',
    '#default_value' => (!empty($key->recipient)) ? $key->recipient : '',
  );
  $form['key_details']['recipient_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Recipient Email'),
    '#description' => t('The email of the person against whom this key is registered'),
    '#required' => TRUE,
    '#maxlength' => '255',
    '#default_value' => (!empty($key->recipient_email)) ? $key->recipient_email : '',
  );
  $form['key_details']['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Active'),
    '#description' => t('Is this key currently active?'),
    '#default_value' => (!empty($key->status)) ? $key->status : TRUE,
  );

  $form['allowed_functions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Allowed function calls for this key'),
  );

  $data = unserialize($key->data);

  module_load_include('inc', 'services', 'services.runtime');
  module_load_include('inc', 'services', 'services.resource_build');

  $resources = _services_build_resources($endpoint->name);
  $class_names = services_operation_class_info();

  // foreach resources: these are:
  // - comment
  // - file
  // - node (etc)
  foreach($resources as $resource_key => $resource) {
    // for this endpoint, is this service allowed:
    if (isset($endpoint->resources[$resource_key])) {
      // the conf for this resource is stored in the endpoint
      $resource_conf = $endpoint->resources[$resource_key];

      // foreach class_names: these are:
      // - operations
      // - actions
      // - relationships
      // - targeted_actions
      foreach ($class_names as $class => $info) {
        // foreach $resource[class]: these are:
        // for 'comment': create, retrieve, update, delete, index
        // for 'comment': create, retrieve, update, delete, index
        if(!isset($resource[$class])) {
          continue;
        }
        foreach ($resource[$class] as $op_name => $op) {
          if (isset($resource_conf[$class][$op_name]['enabled'])) {
            // '_soap_' is hardcoded in _soap_server_get_methods(), so we can do
            // the same here:
            $method_name = $resource_key . '_soap_' . $op_name;

            $form['allowed_functions'][$method_name] = array(
              '#type' => 'checkbox',
              '#title' => $method_name,
              '#description' => (!empty($op['help'])) ? $op['help'] : '',
              '#default_value' => (!empty($data[$method_name])),
            );
          }
        }
      }
    }
  }
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save Key'));

  return $form;
}

/**
 * Submit handler for the edit and add form
 *
 * The form ID is janet_soap_api_edit_key_form for both
 *
 * @param $form
 * @param $form_state
 */
function janet_soap_api_edit_key_form_submit($form, &$form_state) {
  $values = &$form_state['values'];

  if(!empty($values['kid'])) {
    // edit
    $api_key = array(
      'api_key' => $values['api_key'],
      'description' => $values['description'],
      'recipient' => $values['recipient'],
      'recipient_email' => $values['recipient_email'],
      'status' => $values['status'],
      'changed' => REQUEST_TIME,
    );

    $function_permissions = array();

    $methods = _soap_server_get_methods($values['endpoint_name']);
    foreach($methods as $method_name => $method_details) {
      $function_permissions[$method_name] = $values[$method_name];
    }

    $api_key['data'] = serialize($function_permissions);

    db_update('janet_api_keys')
      ->fields($api_key)
      ->condition('kid', $values['kid'])
      ->execute();

    drupal_set_message(t('Key %key updated', array('%key' => $values['api_key'])));
  } else {
    // add
    global $user;
    $api_key = array(
      'api_key' => $values['api_key'],
      'description' => $values['description'],
      'recipient' => $values['recipient'],
      'recipient_email' => $values['recipient_email'],
      'uid' => $user->uid,
      'status' => $values['status'],
      'created' => REQUEST_TIME,
      'changed' => REQUEST_TIME,
    );

    $function_permissions = array();

    $methods = _soap_server_get_methods($values['endpoint_name']);
    foreach($methods as $method_name => $method_details) {
      $function_permissions[$method_name] = $values[$method_name];
    }

    $api_key['data'] = serialize($function_permissions);

    db_insert('janet_api_keys')
      ->fields($api_key)
      ->execute();

    drupal_set_message(t('Key %key updated', array('%key' => $values['api_key'])));
  }
}

/**
 * Delete key confirmation page callback
 *
 * Called by CTOOLS Ajax (Modal) or nojs equivalent
 *
 * @see janet_soap_api_edit_key_form() for actual form
 *
 * @param $server_endpoint
 * @param bool $nojs
 * @param $api_key
 *   object, not string
 * @return string
 */
function janet_soap_api_delete_key($server_endpoint, $js = false, $api_key) {
  $title = t('Delete Key: !key', array('!key' => $api_key->api_key));

  $form_state = array(
    'ajax' => TRUE,
    'title' => $title,
    'server_endpoint' => $server_endpoint,
    'key' => $api_key,
  );

  if ($js) {
    ctools_include('ajax');
    ctools_include('modal');

    $output = ctools_modal_form_wrapper('janet_soap_api_delete_key_form', $form_state);

    if (!empty($form_state['executed'])) {
      $output[] = ctools_ajax_command_reload();
    }
    print ajax_render($output);
  }
  else {
    // Yes, this one could be called with return confirm_form() but
    // it's just easier this way to have the same form for AJAX and
    // non AJAX versions. Modals with confirm_form don't really work.
    drupal_set_title($title);
    return drupal_get_form('janet_soap_api_edit_delete_form', $form_state);
  }
}

/**
 * Form callback for delete form
 *
 * Called from delete function
 * @see janet_soap_api_delete_key()
 *
 * @param $form
 * @param $form_state
 * @param null $extra_values
 * @return array
 */
function janet_soap_api_delete_key_form($form, &$form_state, $extra_values = null) {
  if(empty($form_state['ajax'])) {
    // called as a page
    $key = $extra_values['key'];
    $ajax = false;
  } else {
    // called through ctools modal
    $key = $form_state['key'];
    $ajax = true;
  }

  $form['kid'] = array(
    '#type' => 'value',
    '#value' => $key->kid,
  );
  $form['api_key'] = array(
    '#type' => 'value',
    '#value' => $key->api_key,
  );

  $form['message'] = array(
    '#type' => 'markup',
    '#markup' => t('Are you sure you wish to delete key %key? This action cannot be undone.', array(
      '%key' => $key->api_key,
    )),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete Key'),
  );

  if($ajax) {
    $form['actions']['cancel'] = array(
      '#type' => 'markup',
      '#markup' => l(t('Cancel'), '#', array('attributes' => array('class' => array('ctools-close-modal')))),
    );
  } else {
    $form['actions']['cancel'] = array(
      '#type' => 'markup',
      '#markup' => l(t('Cancel'), $_GET['destination']),
    );
  }

  return $form;
}

/**
 * Submit handler for the edit and add form
 *
 * The form ID is janet_soap_api_edit_key_form for both
 *
 * @param $form
 * @param $form_state
 */
function janet_soap_api_delete_key_form_submit($form, &$form_state) {
  $values = &$form_state['values'];

  if(!empty($values['kid'])) {
    // just to check we don't delete on a zero condition!
    db_delete('janet_api_keys')
      ->condition('kid', $values['kid'])
      ->execute();

    drupal_set_message(t('Key %key deleted', array('%key' => $values['api_key'])));
  }
}

/**
 * Generates a pseudo-random key which is a randomly shuffled 25
 * character long permutation of [0-9A-Z] with - every five
 * characters
 *
 * @return string
 */
function _janet_soap_api_generate_key() {
  $key = substr(chunk_split(substr(str_shuffle("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, 25), 5, '-'), 0, 29);

  // This does a quick check to see if this key is already in the
  // database and assigned. If not, use it. If it is, get a new key
  if(janet_soap_api_key_load_by_field('api_key', $key)) {
    return _janet_soap_api_generate_key();
  }
  return $key;
}
