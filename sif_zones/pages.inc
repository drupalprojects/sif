<?php

/**
 * Zones Page
 */
function sif_zones_page(){

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'sif_data_object')
    ->entityCondition('bundle', 'zone');
  $result = $query->execute();

  $rows = array();

  if (isset($result['sif_data_object'])) {
    $object_ids = array_keys($result['sif_data_object']);
    $zones = entity_load('sif_data_object', $object_ids);
    foreach ($zones as $zone){
      $row = array();
      $row[] = l($zone->id, "sif/$zone->id");
      $row[] = t($zone->description);

      $items = array();
      if (count($zone->zone_properties)) {
        foreach ($zone->zone_properties as $name => $property) {
          $items[] = "$name:<br/>$property";
        }
        $row[] = theme('item_list', array('items' => $items));
      } else {
        $row[] = t('None');
      }

      $actions = array();
      $actions[] = l(t('View XML'), "sif/$zone->id/xml");
      $row[] = implode(' | ', $actions);

      $rows[] = $row;
    }
  }
  $header = array(
    'name',
    t('Description'),
    t('Properties'),
    t('Actions'),
  );

  $build['zones'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('You have no zones in this site.'),
  );

  return $build;
}


/**
 * Get Zones Form
 */
function sif_zones_get(){
  $form['note'] = array(
    '#type' => 'markup',
    '#markup' => '<p>' . t('Load all zones from the server.  This will update any existing zones as well.') . '</p>',
  );
  $form['submit'] = array(
    '#value' => t('GET zones'),
    '#type' => 'submit',
  );

  // Placeholder element for the validate function.
  $form['xml'] = array(
    '#type' => 'value',
  );
  return $form;
}


/**
 * Get Zones Form Submit
 */
function sif_zones_get_submit(){
  $response = sif_get('zone', array('multi' => TRUE));
  $xml = $response->data;
  $zones =  new SimpleXMLElement($xml);
  foreach ($zones->zone as $i => $zone){
    sif_store('zone',  $zone->asXML(), $response->headers);
    $zone_found = TRUE;
  }
  if (!isset($zone_found)){
    drupal_set_message(t('No zones were found.'));
  }
  drupal_goto('admin/sif/zones');
}
