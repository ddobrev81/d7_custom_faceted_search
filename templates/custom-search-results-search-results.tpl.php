<?php

/**
 * @file
 *
 * Available variables:
 * - $search_results:
 * - $module:
 * - $type_of_search:
 * - $
        'response' => NULL,
        'results_title' => NULL,
        'rows_per_page' => NULL,
        'suggested_words' => NULL,
        'searched_keyword' => NULL,
        'image_style' => NULL,
        'enable_see_more' => NULL,
        'see_more_link_text' => NULL,
        'see_more_link_points_to' => NULL,
        'type_of_search' => NULL,
        'pwb_date_formats' => NULL,
 * 
 * 
 * 
 * - $search_results: All results as it is rendered through
 *   search-result.tpl.php
 * - $module: The machine-readable name of the module (tab) being searched, such
 *   as "node" or "user".
 *
 * @see template_preprocess_search_results()
 */
?>

<?php if ($search_results): ?>
  <ol class="list-search <?php print $module; ?>-results">
    <?php print $search_results; ?>
  </ol>

  <?php if (isset($see_more)): ?>
    <p><?php print $see_more; ?></p>
  <?php endif; ?>

  <?php if (isset($pager)): ?>
    <div class="pager"><?php print $pager; ?></div>
  <?php endif; ?>
<?php else : ?>
  <?php print prp_search_results_help('custom_search_results#noresults', drupal_help_arg()); ?>
<?php endif; ?>
