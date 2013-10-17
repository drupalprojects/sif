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

          break;
      }
    }
    return $entities;
  }
}
