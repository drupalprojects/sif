<?php


/**
 * A controller extending EntityAPIController.
 */
class StudentEntityAPIController extends SifEntityAPIController {

  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    // Look for helper function
    $method = "load_" . $entity->type;
    if (method_exists('StudentEntityAPIController', $method)){
      $this->{$method}($entity, $ids, $conditions);
    }
    return $entities;
  }

  /**
   * Load Student
   */
  private function load_student(&$entity, $ids = array(), $conditions = array()){

    // Demographics
    $entity->demographics = array();
    foreach ($entity->data->demographics->children() as $demographic => $s){
      $entity->demographics[(string) $demographic] = (string) $s;
    }

    // Email addresses
    $entity->emailList = array();
    foreach ($entity->data->emailList->children() as $s){
      $entity->emailList[(string) $s->attributes()->emailType] = (string) $s;
    }
   
    // Name
    $entity->name = array();
    foreach ($entity->data->name->nameOfRecord->children() as $type => $s){
      $entity->name[(string) $type] = (string) $s;
    }
  }
}
