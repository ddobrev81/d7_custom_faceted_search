<?php

/**
 * @file
 */

/**
 *
 * @param unknown_type $form
 * @param unknown_type $form_state
 */
function _config_form($form, &$form_state) {
  $form = array();

  $form['_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Custom Search results debug mode'),
    '#default_value' => variable_get('_debug', 0),
  );

  return system_settings_form($form);
}
