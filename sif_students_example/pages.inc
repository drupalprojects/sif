<?php
/**
 * @file
 * Pages for the sif_students_example module
 */

/**
 * Students Page
 */
function sif_students_example_page() {

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'sif_data_object')
    ->entityCondition('bundle', 'student');
  $result = $query->execute();

  $rows = array();

  if (isset($result['sif_data_object'])) {
    $object_ids = array_keys($result['sif_data_object']);
    $students = entity_load('sif_data_object', $object_ids);
    foreach ($students as $student) {
      $row = array();
      $row[] = l($student->name['fullName'], "sif/$student->id");
      $row[] = $student->localId;
      if (isset($student->demographics['sexus'])) {
        $row[] = $student->demographics['sexus'];
      }
      else {
        $row[] = '';
      }

      $actions = array();
      $actions[] = l(t('View XML'), "sif/$student->id/xml");
      $actions[] = l(t('Delete'), "sif/$student->id/delete", array('query' => array('destination' => 'sif/students')));
      $row[] = implode(' | ', $actions);

      $rows[] = $row;
    }
  }
  $header = array(
    'name',
    t('Local ID'),
    t('Sex'),
    t('Actions'),
  );

  $build['students'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('You have no students in this site.'),
  );

  return $build;
}

/**
 * Create Students Form
 */
function sif_students_example_create() {
  $form['#tree'] = TRUE;
  $form['full_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Full Name'),
    '#description' => t('Enter the name you would like to use for this student.'),
    '#required' => TRUE,
  );
  $form['given_name'] = array(
    '#type' => 'textfield',
    '#title' => t('First Name'),
    '#description' => t('Enter the first name you would like to use for this student.'),
    '#required' => TRUE,
  );
  $form['family_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last Name'),
    '#description' => t('Enter the last name you would like to use for this student.'),
    '#required' => TRUE,
  );
  $form['sexus'] = array(
    '#type' => 'radios',
    '#title' => t('Student Sex'),
    '#description' => t('Sex of the student.'),
    '#options' => array(
      'Male' => t('Male'),
      'Female' => t('Female'),
    ),
    '#required' => TRUE,
  );
  $form['local_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Local ID'),
    '#description' => t('Enter the Local ID you would like to use for this student.'),
    '#required' => FALSE,
  );
  $form['emails_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Email Addresses'),
    // Set up the wrapper so that AJAX will be able to replace the fieldset.
    '#prefix' => '<div id="emails-fieldset-wrapper">',
    '#suffix' => '</div>',
  );
  if (empty($form_state['num_emails'])) {
    $form_state['num_emails'] = 1;
  }
  for ($i = 0; $i < $form_state['num_emails']; $i++) {
    $form['emails_fieldset']['email_type'][$i] = array(
      '#type' => 'textfield',
      '#title' => t('Email Type'),
    );
    $form['emails_fieldset']['email'][$i] = array(
      '#type' => 'textfield',
      '#title' => t('Email'),
    );
  }
  $form['emails_fieldset']['add_email'] = array(
    '#type' => 'submit',
    '#value' => t('Add another email address'),
    '#submit' => array('sif_students_example_ajax_add_email'),
    '#ajax' => array(
      'callback' => 'sif_students_example_ajax_add_email_callback',
      'wrapper' => 'emails-fieldset-wrapper',
    ),
  );
  if ($form_state['num_emails'] > 1) {
    $form['emails_fieldset']['remove_email'] = array(
      '#type' => 'submit',
      '#value' => t('Remove email'),
      '#submit' => array('sif_students_example_ajax_remove_email'),
      '#ajax' => array(
        'callback' => 'sif_students_example_ajax_add_email_callback',
        'wrapper' => 'emails-fieldset-wrapper',
      ),
    );
  }

  $form['submit'] = array(
    '#value' => t('POST Student'),
    '#type' => 'submit',
  );

  // Placeholder element for the validate function.
  $form['xml'] = array(
    '#type' => 'value',
  );
  $form['headers'] = array(
    '#type' => 'value',
  );
  return $form;
}

/**
 * Callback for both ajax-enabled buttons.
 *
 * Selects and returns the fieldset with the names in it.
 */
function sif_students_example_ajax_add_email_callback($form, &$form_state) {
  return $form['emails_fieldset'];
}

/**
 * Submit handler for the "add-one-more" button.
 *
 * Increments the max counter and causes a rebuild.
 */
function sif_students_example_ajax_add_email($form, &$form_state) {
  $form_state['num_emails']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "remove one" button.
 *
 * Decrements the max counter and causes a form rebuild.
 */
function sif_students_example_ajax_remove_email($form, &$form_state) {
  if ($form_state['num_emails'] > 1) {
    $form_state['num_emails']--;
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Create Environment validation: POST happens here.
 */
function sif_students_example_create_validate($form, &$form_state) {
  $xml = sif_students_example_create_xml($form_state);
  $results = sif_post('student', $xml);

  if (!$results) {
    form_set_error('xml', t('Error processing request. See above errors.'));
  }
  else {
    form_set_value($form['xml'], $results->data, $form_state);
    form_set_value($form['headers'], $results->headers, $form_state);
  }
}

/**
 * Create Environment submit: entity_save() happens here.
 */
function sif_students_example_create_submit($form, &$form_state) {
  // Save Environment entity.
  $success = sif_store('student', $form_state['values']['xml'], $form_state['values']['headers']);

  if ($success) {
    $form_state['redirect'] = 'sif/students';
  }
}


/**
 * Generate a new environment XML object for POSTing
 */
function sif_students_example_create_xml(&$form_state) {
  $emails = '';
  if (!empty($form_state['num_emails'])) {
    for ($i = 0; $i < $form_state['num_emails']; $i++) {
      $email_type = $form_state['values']['emails_fieldset']['email_type'][$i];
      $email = $form_state['values']['emails_fieldset']['email'][$i];
      $emails .= <<<XML
      <email emailType="$email_type">$email</email>
XML;
    }
  }
  $sexus = $form_state['values']['sexus'];
  $local_id = $form_state['values']['local_id'];
  $family_name = $form_state['values']['family_name'];
  $full_name = $form_state['values']['full_name'];
  $given_name = $form_state['values']['given_name'];
  return <<<XML
<student>
  <demographics>
    <sexus>$sexus</sexus>
  </demographics>
  <emailList>
    $emails
  </emailList>
  <localId>
    $local_id
  </localId>
  <name nameRole="">
    <nameOfRecord>
      <familyName>$family_name</familyName>
      <fullName>$full_name</fullName>
      <givenName>$given_name</givenName>
    </nameOfRecord>
  </name>
</student>
XML;
}


/**
 * Get Students Form
 */
function sif_students_example_get() {
  $form['note'] = array(
    '#type' => 'markup',
    '#markup' => '<p>' . t('Load all students from the server.  This will update any existing students as well.') . '</p>',
  );
  $form['submit'] = array(
    '#value' => t('GET Students'),
    '#type' => 'submit',
  );

  // Placeholder element for the validate function.
  $form['xml'] = array(
    '#type' => 'value',
  );
  return $form;
}


/**
 * Get Students Form
 */
function sif_students_example_get_submit() {
  $student_found = FALSE;
  if ($response = sif_get('student', array('multi' => TRUE))) {
    $xml = $response->data;
    $students = new SimpleXMLElement($xml);
    foreach ($students->student as $i => $student) {
      sif_store('student',  $student->asXML(), $response->headers);
      $student_found = TRUE;
    }
  }
  if (!$student_found) {
    drupal_set_message(t('No students were found.'));
  }
  drupal_goto('sif/students');
}
