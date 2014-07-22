<?php
/**
 * @file
 * Class that extends EntityAPIController to add our customizations
 */

/**
 * A controller extending EntityAPIController.
 */
class SifEntityAPIController extends EntityAPIController {

  /**
   * Extends parent::load() in order to extract important type-specific
   * data.
   */
  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    $types = sif_get_types();
    foreach ($entities as &$entity) {

      // Give all entities a SimpleXML object
      $entity->data = new SimpleXMLElement($entity->xml);

      // Reload headers as array
      $entity->meta = unserialize(strval($entity->headers));

      // Load all desired properties
      foreach ($types[$entity->type]['properties'] as $property_name => $property) {
        $items = $entity->data->xpath('/' . $entity->type . $property['path']);

        switch ($property['type']) {
          case 'single':
            $entity->{$property_name} = strval($items[0]);
            break;

          case 'multi':
            $entity->{$property_name} = array();
            foreach ($items as $item) {
              $key = FALSE;
              if (isset($property['key'])){
                $key = strval($item->attributes()->{$property['key']});
                $entity->{$property_name}[$key] = strval($item);
              }
              else {
                $entity->{$property_name}[$key] = strval($item);
              }
            }
            break;

          case 'tree':
            foreach ($items as $item) {
              $iterator = new SimpleXMLIterator($item->asXML());

              $keys = array();
              foreach ($item->attributes() as $key => $value) {
                $keys[] = $value;
              }
              $key = implode(':', $keys);
              if ($key) {
                $entity->{$property_name}[$key] = $this->tree_iterator($iterator);
              }
              else {
                $entity->{$property_name} = $this->tree_iterator($iterator);
              }
            }
            break;
        }
      }
    }
    return $entities;
  }

  /**
   * Iterates over XML to create an associative array representation
   * @param $iterator
   * @return array
   */
  private function tree_iterator($iterator) {
    $arr = array();
    for ($iterator->rewind(); $iterator->valid(); $iterator->next()) {
      $keys = array();
      foreach ($iterator->current()->attributes() as $key => $value) {
        $keys[] = $value;
      }
      $key = implode(':', $keys);
      if (!$key) {
        $key = strval($iterator->key());
      }
      if ($iterator->hasChildren()) {
        $arr[$key] = $this->tree_iterator($iterator->current());
      }
      else {
        $arr[$key] = strval($iterator->current());
      }
    }
    return $arr;
  }

  /**
   * A clone of EntityAPIController::create
   */
  public function create(array $values = array()) {
    // Add is_new property only if we don't have this entity yet.
    $created = db_query('SELECT created FROM {sif_entity} WHERE id = :id', array(':id' => $values['id']))->fetchField();
    if ($created) {
      $values['created'] = $created;
      $values['changed'] = time();
      $values['is_new'] = FALSE;
    }
    else {
      $values['is_new'] = TRUE;
      $values['created'] = time();
      $values['changed'] = time();
    }
    return parent::create($values);
  }

  /**
   * Extends parent::save to allow setting of default environment.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $saved = parent::save($entity, $transaction);
    if ($saved) {
      // @TODO: move to post_save_TYPE() "hook"
      switch ($entity->type) {
        case 'environment':
          if (!variable_get('sif_default_environment_id')) {
            $data = new SimpleXMLElement($entity->xml);
            variable_set('sif_default_environment_id', strval($data->attributes()->id));
          }

          break;
      }
    }
    return $saved;
  }
}
