<?php

/**
 * @file
 * Plugin to provide a apachesolr_search_results context
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => FALSE,
  'title' => t('Custom search results'),
  'icon' => 'icon_search.png',
  'description' => t('The pane will render the results of a keyword search.'),
  'category' => t('Search panes'),
  'edit form' => 'custom_search_results_results_pane_content_type_edit_form',
  'render callback' => 'custom_search_results_results_pane_content_type_render',
  'admin title' => 'custom_search_results_results_pane_content_type_admin_title',
  'render first' => TRUE,
  'defaults' => array(
    'enable_fields_to_display' => FALSE,
    'fields_to_display' => array(),
    'enable_content_types' => FALSE,
    'content_types' => array(),
    'enable_image_style' => FALSE,
    'image_style' => '',
    'enable_date_formats' => FALSE,
    'date_formats' => '',
    'enable_paging' => FALSE,
    'paging' => 0,
    'enable_see_more' => FALSE,
    'see_more_link_text' => '',
    'see_more_link_points_to' => '',
    'see_more_link_n_results' => 0,
    'enable_sort_options' => FALSE,
    'sort_options' => 'relevance',
    'sort_options_order' => 'desc',
    'no_keyword_search' => FALSE,
    'show_result_content_type' => FALSE,
    'override_empty' => FALSE,
    'empty_title' => '',
    'empty' => '',
    'empty_format' => filter_fallback_format(),
    'override_no_key' => FALSE,
    'no_key_title' => '',
    'no_key' => '',
    'no_key_format' => filter_fallback_format(),
  ),
);

/**
 * Render the custom content type.
 */
function custom_search_results_results_pane_content_type_render($subtype, $conf, $panel_args, $context) {
  $keys = (isset($_GET['keys']) && !empty($_GET['keys'])) ? $_GET['keys'] : NULL;
  $no_key = (isset($conf['no_key_title']) && !empty($conf['no_key_title'])) ? $conf['no_key_title'] : t('Your search yielded no results');

  // Build the content type block.
  $block = new stdClass();
  $block->module = 'custom_search_results';
  $block->delta = 'custom_search_results_pane';

  // Display nothing at all if no keywords were entered.
  if (empty($keys) && $conf['no_keyword_search'] == FALSE) {
    if (!empty($conf['override_no_key'])) {
      $block->title = $no_key;
      $block->content = check_markup($conf['no_key'], $conf['no_key_format'], FALSE);

      return $block;
    }
    else {
      $block->title = $no_key;
      $block->content = theme('custom_search_results_no_keys', array());

      return $block;
    }
  }

  list($results, $response) = custom_search_results_do_search($conf, $keys);

  if (!empty($results)) {
    $block->title = t('Search results');
    $block->content = theme('custom_search_results_search_results', array(
      'results' => $results,
      'response' => $response,
      'searched_keyword' => $keys,
      'configurations' => $conf,
    ));
  }
  else {
    if (empty($conf['override_empty'])) {
      $block->title = t('Your search yielded no results');
      $block->content = theme('custom_search_results_no_results', array());
    }
    else {
      $block->title = $conf['empty_title'];
      $block->content = check_markup($conf['empty'], $conf['empty_format'], FALSE);
    }
  }

  return $block;
}

/**
 * Returns an edit form for custom type settings.
 */
function custom_search_results_results_pane_content_type_edit_form($form, &$form_state) {
  ctools_include('dependent');
  $fields_to_display = array();
  $conf = $form_state['conf'];

  $indexed_fields = custom_search_results_get_apachesolr_fields();
  if ($indexed_fields !== FALSE) {
    foreach ($indexed_fields as $field_machine_name => $field_obj) {
      $fields_to_display[$field_machine_name] = $field_machine_name;
    }

    $form['enable_fields_to_display'] = array(
      '#type' => 'checkbox',
      '#default_value' => $conf['enable_fields_to_display'],
      '#title' => t('Enable fields to display. If not enabled or nothing selected all fields will be displayed.'),
    );

    $form['fields_to_display'] = array(
      '#type' => 'checkboxes',
      '#options' => $fields_to_display,
      '#title' => t(''),
      '#default_value' => $conf['fields_to_display'],
      '#dependency' => array('edit-enable-fields-to-display' => array(1)),
    );
  }

  $search_content_types = array();
  $content_types = node_type_get_types();
  foreach ($content_types as $id => $content_type_obj) {
    $search_content_types[$id] = $content_type_obj->name;
  }

  $form['enable_content_types'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['enable_content_types'],
    '#title' => t('Enable prefilter by content type. Only results from the selected content types will be shown. If nothing selected - there will be no filter. Results of all content types will be shown.'),
  );

  $form['content_types'] = array(
    '#type' => 'checkboxes',
    '#options' => $search_content_types,
    '#title' => t(''),
    '#default_value' => $conf['content_types'],
    '#dependency' => array('edit-enable-content-types' => array(1)),
  );

  $image_styles = image_styles();
  $image_style_names = array();
  foreach ($image_styles as $style_name => $style_settings) {
    $image_style_names[$style_name] = $style_name;
  }

  $form['enable_image_style'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['enable_image_style'],
    '#title' => t('Enable image size preset'),
  );

  $form['image_style'] = array(
    '#type' => 'select',
    '#title' => t('Select image style for the search result image'),
    '#options' => $image_style_names,
    '#default_value' => $conf['image_style'],
    '#dependency' => array('edit-enable-image-style' => array(1)),
  );

  $format_types_raw = system_get_date_types();
  $format_types = array();
  foreach ($format_types_raw as $format_type_item) {
    $format_types[$format_type_item['type']] = $format_type_item['title'];
  }

  $form['enable_date_formats'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['enable_date_formats'],
    '#title' => t('Enable date formats'),
  );

  $form['date_formats'] = array(
    '#type' => 'select',
    '#title' => t('Select date format for the search results.'),
    '#options' => $format_types,
    '#default_value' => $conf['pwb_date_formats'],
    '#dependency' => array('edit-enable-date-formats' => array(1)),
  );

  $form['enable_paging'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['enable_paging'],
    '#title' => t('Enable paging'),
  );

  $form['paging'] = array(
    '#type' => 'textfield',
    '#title' => t('Please set how many results per page would you like? Zero or empty would be considered for unlimited'),
    '#default_value' => $conf['paging'],
    '#dependency' => array('edit-enable-paging' => array(1)),
  );

  $form['enable_see_more'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['enable_see_more'],
    '#title' => t('Enable "See more" link at the bottom of the results. Will overwrite paging settings.'),
  );

  $form['see_more_link_text'] = array(
    '#type' => 'textfield',
    '#default_value' => $conf['see_more_link_text'],
    '#title' => t('See more link text'),
    '#dependency' => array('edit-enable-see-more' => array(1)),
  );

  $form['see_more_link_points_to'] = array(
    '#type' => 'textfield',
    '#default_value' => $conf['see_more_link_points_to'],
    '#title' => t('See more link point to page'),
    '#dependency' => array('edit-enable-see-more' => array(1)),
  );

  $form['see_more_link_n_results'] = array(
    '#type' => 'textfield',
    '#title' => t('Set number of results before the "See more" link? Zero or empty would be considered for unlimited'),
    '#default_value' => $conf['see_more_link_n_results'],
    '#dependency' => array('edit-enable-see-more' => array(1)),
  );

  $form['enable_sort_options'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['enable_sort_options'],
    '#title' => t('Enable sort options'),
  );

  $form['sort_options'] = array(
    '#type' => 'select',
    '#title' => t('Sort search results by:'),
    '#options' => array(
      'relevance' => t('Relevance'),
      'date' => t('Date'),
    ),
    '#default_value' => $conf['sort_options'],
    '#dependency' => array('edit-enable-sort-options' => array(1)),
  );

  $form['sort_options_order'] = array(
    '#type' => 'select',
    '#title' => t('Sort order direction'),
    '#options' => array(
      'asc' => t('ASC'),
      'desc' => t('DESC'),
    ),
    '#default_value' => $conf['sort_options_order'],
    '#dependency' => array('edit-enable-sort-options' => array(1)),
  );

  $form['no_keyword_search'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['no_keyword_search'],
    '#title' => t('Enable "NO keyword" search. Will return all results.'),
  );

  $form['show_result_content_type'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['show_result_content_type'],
    '#title' => t('Enable showing the content type of each search result. The content type will be acting as a filter as well. Clicking on it will filter all search results to the selected content type only.'),
  );

  $form['override_empty'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['override_empty'],
    '#title' => t('Override "no result" text'),
  );

  $form['empty_title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $conf['empty_title'],
    '#dependency' => array('edit-override-empty' => array(1)),
  );

  $form['override_no_key'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['override_no_key'],
    '#title' => t('Display text if no search keywords were submitted'),
  );

  $form['no_key_title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $conf['no_key_title'],
    '#dependency' => array('edit-override-no-key' => array(1)),
  );

  $form['no_key_field'] = array(
    '#type' => 'text_format',
    '#title' => t('No result text'),
    '#default_value' => $conf['no_key'],
    '#format' => $conf['no_key_format'],
    '#dependency' => array('edit-override-no-key' => array(1)),
  );

  return $form;
}

/**
 * Submit handler for search form.
 */
function custom_search_results_results_pane_content_type_edit_form_submit($form, &$form_state) {
  // Copy everything from defaults.
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Returns the administrative title for a type.
 */
function custom_search_results_results_pane_content_type_admin_title($subtype, $conf, $context) {
  if (isset($conf) && !empty($conf)) {
    if (isset($conf['override_title']) && !empty($conf['override_title_text']) && $conf['override_title'] == 1) {
      return t('Pane: ":pane_title"', array(':pane_title' => $conf['override_title_text']));
    }
  }

  return t('Pane: Custom search results');
}
