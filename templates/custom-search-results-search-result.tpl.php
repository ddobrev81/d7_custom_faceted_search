<?php

/**
 * @file

 * Available variables:
 * - $url: URL of the result.
 * - $title: Title of the result.
 * - $snippet: A small preview of the result. Does not apply to user searches.
 * - $info: String of all the meta information ready for print. Does not apply
 *   to user searches.
 * - $info_split: Contains same data as $info, split into a keyed array.
 * - $module: The machine-readable name of the module (tab) being searched, such
 *   as "node" or "user".
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 *
 * Default keys within $info_split:
 * - $info_split['type']: Node type (or item type string supplied by module).
 * - $info_split['user']: Author of the node linked to users profile. Depends
 *   on permission.
 * - $info_split['date']: Last update of the node. Short formatted.
 * - $info_split['comment']: Number of comments output as "% comments", %
 *   being the count. Depends on comment.module.
 *
 * Other variables:
 * - $classes_array: Array of HTML class attribute values. It is flattened
 *   into a string within the variable $classes.
 * - $title_attributes_array: Array of HTML attributes for the title. It is
 *   flattened into a string within the variable $title_attributes.
 * - $content_attributes_array: Array of HTML attributes for the content. It is
 *   flattened into a string within the variable $content_attributes.
 *
 * Since $info_split is keyed, a direct print of the item is possible.
 * This array does not apply to user searches so it is recommended to check
 * for its existence before printing. The default keys of 'type', 'user' and
 * 'date' always exist for node searches. Modules may provide other data.
 * @code
 *   <?php if (isset($info_split['comment'])): ?>
 *     <span class="info-comment">
 *       <?php print $info_split['comment']; ?>
 *     </span>
 *   <?php endif; ?>
 * @endcode
 *
 * To check for all available data within $info_split, use the code below.
 * @code
 *   <?php print '<pre>'. check_plain(print_r($info_split, 1)) .'</pre>'; ?>
 * @endcode
 *
 * @see template_preprocess()
 * @see template_preprocess_search_result()
 * @see template_process()
 */
?>

<li class="<?php print $classes; ?> <?php print (isset($item_class)) ? $item_class : ''; ?>"<?php print $attributes; ?>>
  <?php if ($result_title): ?>
    <div class="list-item-title"><?php print $result_title; ?></div>
  <?php endif; ?>

  <?php if (isset($result_date)): ?>
    <div class="list-item-date-and-user">
      <span class="list-item-date"><?php print $result_date; ?></span>
    </div>

    <?php if (isset($result_content_type)):?>
      <div class="list-item-content-type"><?php print ' | ' . t('Posted in !content_type', array('!content_type' => $result_content_type)); ?></div>
    <?php endif;?>
  <?php endif;?>

  <?php if (isset($result_content_type) && !isset($result_date)) : ?>
    <div class="list-item-content-type"><?php print t('Posted in !content_type', array('!content_type' => $result_content_type)); ?></div>
  <?php endif;?>

  <?php if (isset($result_snippet)): ?>
    <div class="list-item-snippet"><?php print $result_snippet; ?></div>
  <?php endif; ?>

  <?php if (isset($result_websites)): ?>
    <?php foreach ($result_websites as $index => $link) { ?>
      <div class="list-item-snippet"><?php print $link; ?></div>
    <?php } ?>
  <?php endif; ?>
</li>
