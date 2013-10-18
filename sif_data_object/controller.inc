<?php


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
    foreach ($entities as &$entity){

      // Give all entities a SimpleXML object
      $entity->data = new SimpleXMLElement($entity->xml);
      switch ($entity->type){
        case 'environment':
          //String Properties
          $properties = array('sessionToken', 'solutionId', 'defaultZoneId', 'consumerName');
          foreach ($properties as $property){
            $entity->{$property} = (string) $entity->data->{$property};
          }

          // Infrastructure Services
          $entity->infrastructureServices = array();
          foreach ($entity->data->infrastructureServices->infrastructureService as $s){
            $entity->infrastructureServices[(string) $s->attributes()->name] = (string) $s;
          }

          // Authorization key
          $entity->authorizationToken = sif_get_application_key($entity->sessionToken);

          break;
      }
    }
    return $entities;
  }

  /**
   * Extends parent::save to allow setting of default environment.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $saved = parent::save($entity, $transaction);
    if ($saved) {
      switch ($entity->type){
        case 'environment':
          if (!variable_get('sif_default_environment_id')) {
            $data = new SimpleXMLElement($entity->xml);
            variable_set('sif_default_environment_id', (string) $data->attributes()->id);
          }

          break;
      }
    }
    return $saved;
  }
}
