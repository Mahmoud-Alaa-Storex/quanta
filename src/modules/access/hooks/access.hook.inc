<?php
namespace Quanta\Common;

/**
 * Implements hook_shadow_node_edit.
 *
 * @param Environment $env
 *   The current Environment.
 * @param $vars
 *   An array of variables.
 */
function access_shadow_node_permissions(Environment $env, array $vars){
  /** @var Shadow $shadow */
  $shadow = &$vars['shadow'];
  $module_path = $env->getModulePath('access');
  $tab_contents =  file_get_contents($module_path . '/tpl/node_permissions.html');
  $shadow->addTab('manage permissions',$tab_contents, 5);
}

/**
 * Implements hook_node_presave().
 *
 * @param Environment $env
 *   The current Environment.
 * @param $vars
 *   An array of variables.
 */
function access_node_presave(Environment $env, array $vars){
  $node_permissions = array(
    'node_add',
    'node_edit',
    'node_delete',
    'node_view',
  );
  /** @var Node $node */
  $node = &$vars['node'];
  // TODO: review this process.
  if (!isset($node->json->permissions)) {
    $node->json->permissions = new \stdClass();
  }
  // Set all the node's permissions.
  foreach ($node_permissions as $permission) {
    if (isset($vars['data'][$permission])) {
      $node->json->permissions->{$permission} = $vars['data'][$permission];
    }
  }
}

/**
 * Implements hook_node_open().
 *
 * @param Environment $env
 *   The Environment.
 * @param $vars
 *   An array of variables.
 */
function access_node_open(Environment $env, array &$vars) {
  /** @var Node $node */
  $node = &$vars['node'];

  // Check if the user can access the node.
  if (!empty($node) && $node->exists) {
    $has_access = NodeAccess::check($env, \Quanta\Common\Node::NODE_ACTION_VIEW, array('node' => $node));
    if (!$has_access) {
      $vars['node']->forbidden = TRUE;
    }
  }
}

/**
 * Check if a qtag is filtered via the filter attribute.
 *
 * TODO: make it open for all node attributes
 *
 * @param Environment $env
 *   The Environment.
 * @param $vars
 *   An array of variables.
 */
function access_qtag_preload(Environment $env, array $vars) {
  $node = NodeFactory::current($env);
  $user = UserFactory::current($env);
  $filter_key = null;
  /** @var Qtag $qtag */
  $qtag = &$vars['qtag'];
  if(!empty($qtag->getAttribute('filter_node'))){
    $node = NodeFactory::load($env,$qtag->getAttribute('filter_node'));
  }
  if(!empty($qtag->getAttribute('filter_key'))){
    $filter_key = $qtag->getAttribute('filter_key');
  }
  // The user has selected to show the tag only if a certain criteria is matched.
  if (!empty($qtag->getAttribute('filter'))) {
    $filter_match = _access_filter($env, $qtag->getAttribute('filter'), $node, $user, $filter_key);
    // If there is an active tag filter, and it doesn't match, empty the tag.
    if (!$filter_match) {
      $qtag->setAccess(FALSE);
    }
  }
}

/**
 * Check access as defined by a qtag filter attribute.
 *
 * @param Environment $env
 *   The Environment.
 * @param string $criteria
 *   The criteria to use for filtering check.
 * @param Node $node
 *   The node to use for filtering check.
 * @param User $user
 *   The user to use for filtering check.
 * @return bool
 *   Returns true if the access check was positive.
 */
function _access_filter($env, $criteria, $node, $user = NULL, $custom_value = null) {

  // Explode the criteria values.
  $filter = explode(',', $criteria);

  // Make matching false by default.
  $filter_match = FALSE;

  if ($user == NULL) {
    $user = UserFactory::load($env, $node->getName());
  }

  $criteria_match = array();
  $i = 0;
  // Check all filters.
  foreach ($filter as $filter_item) {
    if($filter_item == null){
      continue;
    }
    // Count the occurrences of '@'
    $at_count = substr_count($filter_item, '@');
    // If there's more than one '@', split at the first occurrence
    if ($at_count > 1) {
        $first_at_position = strpos($filter_item, '@');
        $filter_type = substr($filter_item, 0, $first_at_position);
        $filter_values = explode(';', substr($filter_item, $first_at_position + 1));
      } else {
        // Otherwise, split at the first ';' occurrence
        $exp = explode('@', $filter_item);
        if(!isset($exp[0]) || !isset($exp[1])){
          continue;
        }
        $filter_type = $exp[0];
        $filter_values = explode(';', $exp[1]);
    }

    if (substr($filter_type, 0, 12) == 'current_user') {
      $filter_type = str_replace('current_user', 'user', $filter_type);
      $user = UserFactory::current($env);
    }
    if (substr($filter_type, 0, 12) == 'node_author') {
      $filter_type = str_replace('node_author', 'user', $filter_type);
      $user = new User($env, $node->getAuthor());
    }

    $filter_match = FALSE;
    $break = FALSE;
    $one_match = FALSE;
 
    switch ($filter_type) {
      // Filter by HTTP referer.
      case 'referer':
        $explode = explode('/', $_SERVER['HTTP_REFERER']);
        $filter_check = $explode;
        break;
      // Filter by current node path.
      case 'author':
        $filter_check = array($node->getAuthor());
        break;
      // Filter by current node path.
      case 'path':
        $filter_check = array($node->getName());
        break;
      // Filter by current node path.
      case 'language':
        $filter_check = array(Localization::getLanguage($env));
        break;
      // Filter by current node's father node.
      case 'father':
        $filter_check = array($node->getFather()->getName());
        break;
      // Filter by current user's name.
      case 'user_name':
        $filter_check = array($user->getName());
        break;
      case 'user_can_edit':
        $filter_check = array('user_can_edit');
        break;
      // Filter by current user's role.
			case 'status':
        $filter_check = array($node->getStatus());
				break;
			case 'user_role':
        $filter_check = $user->getRoles();
        break;
      // Filter by node's lineage (father, grandfather, and all ancestors!).
      case 'lineage':
        $one_match = TRUE;
        $filter_check = array();
        $lineage = $node->getLineage();

        foreach ($lineage as $lineage_node) {
          /** @var Node $lineage_node */
          $filter_check[] = $lineage_node->getName();
        }
        $filter_check[] = $node->getName();

        break;

      case 'context':
        $filter_check[] = $env->getContext();
        break;

      case 'category':
        $filter_check = array();
        $categories = $node->getCategories();
        foreach ($categories as $category_node) {
          /** @var Node $category_node */
          $filter_check[] = $category_node->getName();
        }
        break;

      case 'thumbnail':
        $filter_check = array($node->getThumbnail());
        break;

      case 'file_name':
        $filter_check = array($custom_value);
        break;

      case 'env':
        $filter_check = array($env->getData($custom_value));
        break;

      default:
        if (!empty($filter_type) && !empty($node->getAttributeJSON($filter_type)) || (in_array(\Quanta\Common\Access::EMPTY_FILTER,$filter_values))) {
		      $filter_check[] = $node->getAttributeJSON($filter_type);
        }
        break;
    }

    if (!empty($filter_check)) {       
      // Check if the filtered item corresponds to the user input.
      foreach ($filter_check as $check) {
        foreach ($filter_values as $filter_value) {
          if (substr($filter_value, 0, 1) == '!') {
            $negate = TRUE;
            $filter_value = str_replace('!', '', $filter_value);
          }
          // If comparing with an array of values, check if the value is in the array.
          if (is_array($check) && $filter_value != \Quanta\Common\Access::EMPTY_FILTER) {   
            $filter_match = in_array($filter_value, $check);
          }
          elseif($filter_value == \Quanta\Common\Access::EMPTY_FILTER){   
            $filter_match = !$check; 
          }elseif($check == 'user_can_edit'){
            $filter_match = \Quanta\Common\NodeAccess::check($env,\Quanta\Common\Node::NODE_ACTION_EDIT,array('node' => \Quanta\Common\NodeFactory::load($env,$filter_value)));
          }
	  else {
	    if (!empty($check)) {
        $check = trim($check);
	    } 
	    if (!empty($filter_value)) {
	      $filter_value = trim($filter_value);
	    } 
            $filter_match = ($check == $filter_value) ? true : false;
          }
          if ($filter_match && !isset($negate)) {
            unset($one_match);
            $break = TRUE;
          }

          if (isset($negate)) {
            $filter_match = !$filter_match;
            unset($negate);

            if (!$filter_match) {
              $break = TRUE;
            }
          }

          if ($break) {
            break;
          }
        }
        // For multiple fields such as lineage one match is enough to validate.
        if ($break) {
          break;
        }
      }
    
      //TODO: handle invalid / unknown filters.
      //TODO: use hook for custom filters.

      if (!$filter_match) {
        break;
      }
    }
    else {
	    $filter_match = FALSE;
	    break;
    }

    $criteria_match[$i] = $filter_match;
    $i++;
  }
  return $filter_match;
}
