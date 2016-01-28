<?php

/**
 * @file
 * The standard URL processor.
 */

/**
 * Extension of FacetapiUrlProcessorStandard.
 * Restricts only one value per facet to be selected
 */
class FacetapiUrlProcessorSingleItem extends FacetapiUrlProcessorStandard {

  /**
   * Implements FacetapiUrlProcessor::getQueryString().
   */
  public function getQueryString(array $facet, array $values, $active) {
    $realms = facetapi_get_realm_info();
    $qstring = $this->getParams();
    $active_items = $this->adapter->getActiveItems($facet);
    $field_alias = rawurlencode($facet['field alias']);

    foreach ($realms as $realm_name => $realm) {
      $facet_settings[$realm_name] = $this->adapter->getFacetSettings($facet, $realm);
      $facet_settings_global[$realm_name] = $this->adapter->getFacetSettingsGlobal($facet);

      if ($facet_settings[$realm_name]->settings['widget'] == 'custom_facetapi_links' && $facet_settings_global[$realm_name]->settings['query_type'] == 'term' && $facet_settings[$realm_name]->settings['single_item'] == '1') {
        // Appends to qstring if inactive
        foreach ($values as $value) {
          if (!$active && $value != 'none') {
            $check_query = $this->checkQueryString($field_alias, $qstring[$this->filterKey]);
            if ($check_query !== FALSE) {
              unset($qstring[$this->filterKey][$check_query]);
              $qstring[$this->filterKey] = array_values($qstring[$this->filterKey]);
            }
            $qstring[$this->filterKey][] = $field_alias . ':' . $value;
          }
          else {
            if ($value === 0) {
              $check_query = $this->checkQueryString($field_alias, $qstring[$this->filterKey]);
              if ($check_query !== FALSE) {
                unset($qstring[$this->filterKey][$check_query]);
                $qstring[$this->filterKey] = array_values($qstring[$this->filterKey]);
              }
            }
            else {
              unset($qstring[$this->filterKey][$active_items[$value]['pos']]);
              $qstring[$this->filterKey] = array_values($qstring[$this->filterKey]);
            }
          }
        }
      }
      else {
        $qstring = $this->params;
        // Appends to qstring if inactive, removes if active.
        foreach ($values as $value) {
          if ($facet_settings[$realm_name]->settings['view_all_link'] == '1') {
            if ($active && (isset($active_items[$value]) || $value === 0)) {
              if ($value === 0) {
                $check_query = $this->checkQueryString($field_alias, $qstring[$this->filterKey]);
                if ($check_query !== FALSE) {
                  unset($qstring[$this->filterKey][$check_query]);
                  $qstring[$this->filterKey] = array_values($qstring[$this->filterKey]);
                }
              }
              else {
                unset($qstring[$this->filterKey][$active_items[$value]['pos']]);
                $qstring[$this->filterKey] = array_values($qstring[$this->filterKey]);
              }
            }
            elseif (!$active) {
              $qstring[$this->filterKey][] = $field_alias . ':' . $value;
            }
          }
          else {
            if ($active && isset($active_items[$value])) {
              unset($qstring[$this->filterKey][$active_items[$value]['pos']]);
              $qstring[$this->filterKey] = array_values($qstring[$this->filterKey]);
            }
            elseif (!$active) {
              $qstring[$this->filterKey][] = $field_alias . ':' . $value;
            }
          }
        }
      }
    }

    return array_filter($qstring);
  }

  /**
   * Implements FacetapiUrlProcessor::getQueryString().
   */
  private function checkQueryString($field_alias, array $query_f) {
    foreach ($query_f as $position => $item) {
      if (strpos($item, $field_alias) !== FALSE) {
        return $position;
      }
    }

    return FALSE;
  }

  /**
   * Implements FacetapiUrlProcessor::setBreadcrumb().
   */
  public function setBreadcrumb() {
    $breadcrumb = drupal_get_breadcrumb();
    $env_id = apachesolr_default_environment();
    $apachesolr_has_searched = apachesolr_has_searched($env_id, TRUE);
    $searcher = reset(array_keys(facetapi_get_searcher_info()));
    $response = apachesolr_static_response_cache($searcher);
    $active_items = $this->adapter->getAllActiveItems();
    $last = end($active_items);
    $keys = $_GET['keys'];
    $query = $this->params;
    unset($query[$this->filterKey]);
    if ($apachesolr_has_searched && $response) {

      $total_search_results = (!empty($response)) ? $response->response->numFound : '';
      if ($active_items) {
        if ($keys) {
      // The last item should be text, not a link.
            $breadcrumb[] = $active_items ? l(t('All results'), current_path(), array('query' => $query)) : check_plain($keys);
        }
        foreach ($active_items as $item) {
          $query[$this->filterKey][] = rawurlencode($item['field alias']) . ':' . $item['value'];

          // Replaces with the mapped value.
          $value = $this->adapter->getMappedValue($item['facets'][0], $item['value']);

          // The last item should be text, not a link.
          if ($last == $item) {
            $breadcrumb[] = !empty($value['#html']) ? $value['#markup'] . " ($total_search_results)"  : check_plain($value['#markup']) . " ($total_search_results)";
          }
          else {
            // Appends the filter to the breadcrumb trail.
            $breadcrumb[] = l($value['#markup'], current_path(), array('query' => $query, 'html' => !empty($value['#html'])));
          }
        }
      }
      else {
        $breadcrumb[] = t('All Results (!total results)', array('!total' => $total_search_results));
      }
    }
    else {
      $breadcrumb[] = t('Search');
    }

    drupal_set_breadcrumb($breadcrumb);
  }

}