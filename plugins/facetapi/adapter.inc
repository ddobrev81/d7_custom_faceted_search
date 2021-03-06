<?php

/**
 * @file
 * Classes used by the Facet API module.
 */

/**
 * Facet API adapter for the Pwb Search module.
 */
class FacetapiCustomSearchResultsFacetapiAdapter extends ApacheSolrFacetapiAdapter {

  /**
   * Constructor, sets searcher and type of content being indexed.
   *
   * @param array $searcher_info
   *   The searcher definition.
   */
  public function __construct(array $searcher_info) {
    $this->info = $searcher_info;

    // Registers the query type plugins classes associated with this adapter.
    $registered_types = array();
    foreach (ctools_get_plugins('facetapi', 'query_types') as $plugin) {
      if ($searcher_info['adapter'] == $plugin['handler']['adapter'] || ($searcher_info['adapter'] == 'custom_search_results_adapter' && $plugin['handler']['adapter'] == 'apachesolr')) {
        $type = call_user_func(array($plugin['handler']['class'], 'getType'));
        $registered_types[$type] = $plugin['handler']['class'];
      }
    }

    // Iterates over facets and registers query type plugins.
    foreach ($this->getEnabledFacets() as $facet) {

      // Gets widget type from setting if there are more than one available.
      if (1 == count($facet['query types'])) {
        $query_type = $facet['query types'][0];
      }
      else {
        $settings = $this->getFacetSettingsGlobal($facet)->settings;
        $query_type = !empty($settings['query_type']) ? $settings['query_type'] : FALSE;
      }

      // If we found a query type, register the query type plugin.
      if ($query_type && isset($registered_types[$query_type])) {
        $plugin = new $registered_types[$query_type]($this, $facet);
        $this->queryTypes[$facet['name']] = $plugin;
      }
      else {
        $this->queryTypes[$facet['name']] = FALSE;
      }
    }

    // Instantiates URL processor plugin.
    $id = $searcher_info['url processor'];
    $class = ctools_plugin_load_class('facetapi', 'url_processors', $id, 'handler');
    if (!$class) {
      $class = ctools_plugin_load_class('facetapi', 'url_processors', 'standard', 'handler');
    }
    $this->urlProcessor = new $class($this);

    // Fetches, normalizes, and sets filter params.
    $filter_key = $this->urlProcessor->getFilterKey();
    $params = $this->urlProcessor->fetchParams();
    $this->setParams($params, $filter_key);
  }

  /**
   * Initializes facet builds, adds breadcrumb trail.
   */
  public function processFacets() {
    if (!$this->processed) {
      $this->processed = TRUE;

      // Initializes each facet's render array.
      foreach ($this->getEnabledFacets() as $facet) {
        $processor = new PrpSearchResultsFacetProcessor($this->getFacet($facet));
        $this->processors[$facet['name']] = $processor;
        $this->processors[$facet['name']]->process();
      }

      // Sets the breadcrumb trail if a search was executed.
      if ($this->searchExecuted()) {
        $this->urlProcessor->setBreadcrumb();
      }
    }
  }

  /**
   * Builds the render array for facets in a realm.
   *
   * @param string $realm_name
   *   The machine readable name of the realm.
   *
   * @return array
   *   The render array.
   */
  public function buildRealm($realm_name) {
    // Bails if realm isn't valid.
    // @todo Call watchdog()?
    if (!$realm = facetapi_realm_load($realm_name)) {
      return array();
    }

    // Makes sure facet builds are initialized.
    $this->processFacets();

    // Adds JavaScript, initializes render array.
    drupal_add_js(drupal_get_path('module', 'facetapi') . '/facetapi.js');
    $build = array(
      '#adapter' => $this,
      '#realm' => $realm,
    );

    // Builds each facet in the realm, merges into realm's render array.
    foreach ($this->getEnabledFacets($realm['name']) as $facet) {
      // Continue to the next facet if this one failed its dependencies.
      if (!isset($this->dependenciesPassed[$facet['name']]) ||
          !$this->dependenciesPassed[$facet['name']]) {
        continue;
      }

      // Gets the initialized build.
      $field_alias = $facet['field alias'];
      $processor = $this->processors[$facet['name']];
      $facet_build = $this->getFacet($facet)->build($realm, $processor);

      // Tries to be smart when merging the render arrays. Crazy things happen
      // when merging facets with the same field alias such as taxonomy terms in
      // the fieldset realm. We want to merge only the values.
      foreach (element_children($facet_build) as $child) {
        // Bails if there is nothing to render.
        if (!element_children($facet_build[$child])) {
          continue;
        }
        // Attempts to merge gracefully.
        if (!isset($build[$child])) {
          $build = array_merge_recursive($build, $facet_build);
        }
        else {
          if (isset($build[$child][$field_alias]) && isset($facet_build[$child][$field_alias])) {
            $build[$child][$field_alias] = array_merge_recursive(
                $build[$child][$field_alias], $facet_build[$child][$field_alias]
            );
          }
          elseif (isset($build[$child]['#options']) && isset($facet_build[$child]['#options'])) {
            $build[$child]['#options'] = array_merge_recursive(
                $build[$child]['#options'], $facet_build[$child]['#options']
            );
          }
          else {
            $build = array_merge_recursive($build, $facet_build);
          }
        }
      }
    }

    return $build;
  }

}

/**
 * Processes facets, initializes the build.
 */
class CustomSearchResultsFacetProcessor extends FacetapiFacetProcessor {

  /**
   * Processes the facet items.
   */
  public function process() {
    $this->build = array();
    // Only initializes facet if a query type plugin is registered for it.
    // NOTE: We don't use the chaining pattern so the methods can be tested.
    if ($this->facet->getAdapter()->getFacetQuery($this->facet->getFacet())) {
      $this->build = $this->initializeBuild($this->build);
      $this->build = $this->mapValues($this->build);
      if ($this->build) {
        $settings = $this->facet->getSettings();
        if (!$settings->settings['flatten']) {
          $this->build = $this->processHierarchy($this->build);
        }
        $this->processQueryStrings($this->build);
      }
    }
  }

  /**
   * Initializes the render array's query string variables.
   *
   * @param array &$build
   *   The initialized render array.
   */
  protected function processQueryStrings(array &$build) {
    $facet_settings = $this->facet->getSettings('block')->settings;
    if (isset($facet_settings['view_all_link']) && $facet_settings['view_all_link'] === 1) {
      $counts = '';
      $query_f = array();
      foreach ($build as $value => $item) {
        $counts += $item['#count'];
        if (isset($item['#query']['f']) && !empty($item['#query']['f'])) {
          foreach ($item['#query']['f'] as $index => $f_parameter) {
            $query_f['f'][] = $f_parameter;
          }
        }
      }

      $adapter = $this->facet->getAdapter();

      $build[0] = array(
        '#markup' => t('All results'),
        '#path' => $this->facet->getAdapter()->getSearchPath(),
        '#html' => FALSE,
        '#indexed_value' => 0,
        '#count' => $counts,
        '#active' => !$adapter->getActiveItems($this->facet->getFacet()),
        '#item_parents' => array(),
        '#item_children' => array(),
        '#query' => array(),
      );
    }

    foreach ($build as $value => &$item) {
      $values = array($value);
      // Calculate paths for the children.
      if (!empty($item['#item_children'])) {
        $this->processQueryStrings($item['#item_children']);
        // Merges the childrens' values if the item is active so the children
        // are deactivated along with the parent.
        if ($item['#active']) {
          $values = array_merge(facetapi_get_child_values($item['#item_children']), $values);
        }
      }
      // Stores this item's active children so we can deactivate them in the
      // current search block as well.
      $this->activeChildren[$value] = $values;

      // Formats path and query string for facet item, sets theme function.
      $item['#path'] = $this->getFacetPath($values, $item['#active']);
      $item['#query'] = $this->getQueryString($values, $item['#active']);
    }
  }

}
