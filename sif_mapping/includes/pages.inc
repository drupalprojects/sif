<?php
/**
 * @file
 * Pages for the sif_mapping module
 */

/**
 * Mapping Page
 */
function sif_mapping_list() {
  // Grab the lists so we can get the display versions
  $sif_fields = sif_get_types();
  $node_fields = _sif_mapping_get_node_fields();

  // Grab the mappings that have been defined
  $mappings = sif_mapping_get_mappings();

  $build = array();
  foreach ($mappings as $mapping) {
    $mid = $mapping->mid;
    $actions = array(
      l(t('Add Fields'), "admin/sif/mapping/field/$mid/add", array('query' => array('destination' => 'admin/sif/mapping'))),
      l(t('Delete'), "admin/sif/mapping/map/$mid/delete", array('query' => array('destination' => 'admin/sif/mapping'))),
    );
    $actions = implode('&nbsp;|&nbsp;', $actions);

    $header = array(
      $sif_fields[$mapping->source_type]['name'] . ' (Source)',
      $node_fields[$mapping->destination_type]->label . ' (Destination)',
      $node_fields[$mapping->destination_type]->fields[$mapping->unique_id] . ' (Unique ID)',
      $actions,
    );

    $rows = array();
    foreach ($mapping->fields as $fid => $data) {
      $actions = array(
        l(t('Edit'), "admin/sif/mapping/field/$fid/edit", array('query' => array('destination' => 'admin/sif/mapping'))),
        l(t('Delete'), "admin/sif/mapping/field/$fid/delete", array('query' => array('destination' => 'admin/sif/mapping'))),
      );
      $actions = implode('&nbsp;|&nbsp;', $actions);
      $rows[] = array(
        'data' => array($data->source_field, $node_fields[$mapping->destination_type]->fields[$data->destination_field], '', $actions),
      );
    }
    $build[$mapping->source_type] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('There are no mappings created yet.'),
    );
  }

  // Return the table
  return $build;
}

function sif_mapping_create_form($form, &$form_state) {
  $sif_types = sif_get_types();
  $entity_types = _sif_mapping_get_node_fields();

  $sif_objects = $entity_objects = array();
  foreach ($sif_types as $type => $info) {
    $sif_objects[$type] = $info['name'];
  }
  foreach ($entity_types as $type => $info) {
    $entity_objects[$type] = $info->label;
  }

  $selected = isset($form_state['values']['destination']) ? $form_state['values']['destination'] : key($entity_objects);

  $form['source'] = array(
    '#title' => t('SIF Source'),
    '#type' => 'select',
    '#description' => t('The SIF Object you want to map from.'),
    '#options' => $sif_objects,
  );

  $form['destination'] = array(
    '#title' => t('Entity Destination'),
    '#type' => 'select',
    '#description' => t('The Entity you want to map to.'),
    '#options' => $entity_objects,
    '#default_value' => $selected,
    '#ajax' => array(
      'callback' => '_sif_mapping_create_ajax_callback',
      'wrapper' => 'unique',
      'effect' => 'fade',
    ),
  );

  $form['unique_id'] = array(
    '#title' => t('Unique ID Field'),
    '#type' => 'select',
    '#description' => t('The Entity Field that contains the unique RefID.'),
    '#options' => $entity_types[$selected]->fields,
    '#prefix' => '<div id="unique">',
    '#suffix' => '</div>',
  );

  $form['submit'] = array(
    '#value' => 'Save',
    '#type' => 'submit',
  );

  return $form;
}

/**
 * AJAX callback to handle dynamic setting of the select box
 */
function _sif_mapping_create_ajax_callback($form, &$form_state) {
  return $form['unique_id'];
}

/**
 * Validate the mapping creation form
 */
function sif_mapping_create_form_validate($form, &$form_state) {
  $source_type = check_plain($form_state['values']['source']);

  $mid = db_select('sif_mapping_types', 't')
    ->fields('t', array('mid'))
    ->condition('source_type', $source_type, '=')
    ->execute()
    ->fetchField();

  if ($mid !== FALSE) {
    form_set_error('source', t('This SIF Source is already mapped.'));
  }
}

/**
 * Submit the mapping creation form
 */
function sif_mapping_create_form_submit($form, &$form_state) {
  $source = check_plain($form_state['values']['source']);
  $destination = check_plain($form_state['values']['destination']);
  $unique_id = check_plain($form_state['values']['unique_id']);

  $mid = db_insert('sif_mapping_types')
    ->fields(array(
      'source_type' => $source,
      'destination_type' => $destination,
      'unique_id' => $unique_id,
    ))
    ->execute();
}

/**
 * Create Mapping Form
 */
function sif_mapping_field_form($form, &$form_state, $id, $action) {
  $source_default = $destination_default = '';

  if ($action == 'add') {
    $mapping = sif_mapping_get_type_mapping($id);
  }
  if ($action == 'edit') {
    $mapping = sif_mapping_get_field_mapping($id);
    $source_default = $mapping['source_field'];
    $destination_default = $mapping['destination_field'];
  }

  $source_type = $mapping['source_type'];
  $destination_type = $mapping['destination_type'];

  $sif_types = sif_get_types();
  $entity_types = _sif_mapping_get_node_fields();

  $sif_fields = $entity_fields = array();
  foreach ($sif_types[$source_type]['properties'] as $type => $info) {
    $sif_fields[$type] = $type;
  }
  foreach ($entity_types[$destination_type]->fields as $type => $label) {
    $entity_fields[$type] = $label;
  }

  $form['source'] = array(
    '#title' => t('SIF Source Field'),
    '#type' => 'select',
    '#description' => t('The SIF Property you want to map from.'),
    '#options' => $sif_fields,
    '#default_value' => $source_default,
  );

  $form['destination'] = array(
    '#title' => t('Entity Destination Field'),
    '#type' => 'select',
    '#description' => t('The Entity Field you want to map to.'),
    '#options' => $entity_fields,
    '#default_value' => $destination_default,
  );

  $form['id'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  $form['action'] = array(
    '#type' => 'value',
    '#value' => $action,
  );

  $form['submit'] = array(
    '#value' => 'Save',
    '#type' => 'submit',
  );

  return $form;
}

/**
 * Stores the mappings in the DB
 *
 * @param $form
 * @param $form_state
 */
function sif_mapping_field_form_submit($form, &$form_state) {
  $source_field = check_plain($form_state['values']['source']);
  $destination_field = check_plain($form_state['values']['destination']);
  $id = check_plain($form_state['values']['id']);
  $action = check_plain($form_state['values']['action']);

  if ($action == 'add') {
    db_insert('sif_mapping_fields')
      ->fields(array(
        'mid' => $id,
        'source_field' => $source_field,
        'destination_field' => $destination_field,
      ))
      ->execute();
  }
  if ($action == 'edit') {
    db_update('sif_mapping_fields')
      ->fields(array(
        'source_field' => $source_field,
        'destination_field' => $destination_field,
      ))
      ->condition('fid', $id, '=')
      ->execute();
  }

  return;
}

/**
 * Creates the list of SIF objects and fields to use as sources.
 *
 * @return array List of fields defined in SIF objects
 */
function _sif_mapping_get_sif_fields() {
  // Grab defined SIF objects
  $sif_objects = sif_get_types();

  $sif_fields = array();
  foreach ($sif_objects as $sif_name => $sif_object) {
    // Create an array like 'source_type:source_field' => 'source_type > source_field'
    if (is_array($sif_object['properties'])) {
      foreach ($sif_object['properties'] as $property => $info) {
        $sif_fields[$sif_name][] = array('name' => $property, 'label' => $info['name']);
      }
    }
  }
  return $sif_fields;
}

/**
 * Creates a list of all node types (and user) with fields to use as destinations.
 * Excludes Field Collections for now, as they are problematic to implement.
 *
 * @return array List of node types (and user) with all fields
 */
function _sif_mapping_get_node_fields() {
  // Grab node types and add 'user'
  $node_types = node_type_get_types();
  $node_types['user'] = new stdClass();
  $node_types['user']->name = 'User';

  $node_fields = array();
  foreach ($node_types as $node_type => $info) {
    $entity_type = 'node';
    if ($node_type == 'user') {
      $entity_type = 'user';
    }

    $node_fields[$node_type] = new stdClass();
    $node_fields[$node_type]->label = $info->name;
    // Grab the full list of fields for each node type / user
    $wrapper = entity_metadata_wrapper($entity_type, NULL, array('bundle' => $node_type));
    $field_names = $wrapper->getPropertyInfo();
    ksort($field_names); // Alphabetize for ease of finding

    // Create an array like 'destination_type:destination_field' => 'destination_type > destination_field'
    foreach ($field_names as $field_name => $data) {
      if (!isset($data['type']) || $data['type'] != 'field_collection_item') {
        $node_fields[$node_type]->fields[$field_name] = $data['label'];
      }
    }
  }

  return $node_fields;
}

/**
 * DELETE sif mapping form.
 */
function sif_mapping_field_delete_form($form, &$form_state, $fid) {
  $entity_types = _sif_mapping_get_node_fields();

  $mapping = sif_mapping_get_field_mapping($fid);

  $source = $mapping['source_field'];
  $destination = $entity_types[$mapping['destination_type']]->fields[$mapping['destination_field']];

  $form['fid'] = array(
    '#type' => 'value',
    '#value' => $fid,
  );

  $description = t('Are you sure you want to delete the %source -> %destination field map?', array(
    '%source' => $source,
    '%destination' => $destination,
  ));
  $description .= '<br/>' . t('This action cannot be undone.');

  return confirm_form(
    $form,
    t('Mapping Delete'),
    'admin/sif/mapping',
    $description,
    t('Delete')
  );
}

/**
 * DELETE sif object form submission
 */
function sif_mapping_field_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $fid = $form_state['values']['fid'];

    db_delete('sif_mapping_fields')
      ->condition('fid', $fid)
      ->execute();
  }
}

/**
 * DELETE sif mapping form.
 */
function sif_mapping_type_delete_form($form, &$form_state, $mid) {
  $sif_types = sif_get_types();
  $entity_types = _sif_mapping_get_node_fields();

  $mappings = sif_mapping_get_map_mapping($mid);

  $source = $sif_types[$mappings[0]['source_type']]['name'];
  $destination = $entity_types[$mappings[0]['destination_type']]->label;

  $form['mid'] = array(
    '#type' => 'value',
    '#value' => $mid,
  );

  $description = t('Are you sure you want to delete the %source -> %destination field map?', array(
    '%source' => $source,
    '%destination' => $destination,
  ));

  $items['items'] = array();
  foreach ($mappings as $mapping) {
    if ($mapping['source_field'] !== NULL) {
      $source = $mapping['source_field'];
      $destination = $entity_types[$mapping['destination_type']]->fields[$mapping['destination_field']];
      $items['items'][] = "$source -> $destination";
    }
  }
  if (count($items['items'])) {
    $description .= '<br/>' . t('This will also delete the following Field Mappings:');
    $description .= theme('item_list', $items);
  }

  $description .= '<br/>' . t('This action cannot be undone.');

  return confirm_form(
    $form,
    t('Mapping Delete'),
    'admin/sif/mapping',
    $description,
    t('Delete')
  );
}

/**
 * DELETE sif object form submission
 */
function sif_mapping_type_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $mid = check_plain($form_state['values']['mid']);

    db_delete('sif_mapping_fields')
      ->condition('mid', $mid)
      ->execute();

    db_delete('sif_mapping_types')
      ->condition('mid', $mid)
      ->execute();
  }
}
