<?php
namespace Quanta\Common;

/**
 * Implementation of hook_action_node_add.
 * There is a node add request.
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_action_node_add(Environment $env, array $vars){

  $response_json = NodeFactory::requestAction($env, $vars['action'], $vars['data']);
  exit($response_json);
}

/**
 * Implements hook_action_node_edit.
 * There is a node edit request.
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_action_node_edit(Environment $env, array $vars){
  $response_json = NodeFactory::requestAction($env, $vars['action'], $vars['data']);
  exit($response_json);
}

/**
 * Implements hook_action_node_duplicate.
 * There is a node duplicate request.
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_action_node_duplicate(Environment $env, array $vars){
  $response_json = NodeFactory::requestAction($env, $vars['action'], $vars['data']);
  exit($response_json);
}

/**
 * Implements hook_action_node_change_author.
 * There is a node change author request.
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_action_node_change_author(Environment $env, array $vars){
  $response_json = NodeFactory::requestAction($env, $vars['action'], $vars['data']);
  exit($response_json);
}


/**
 * Implements hook_action_node_delete.
 * There is a node delete request.
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_action_node_delete(Environment $env, array $vars){
	$response_json = NodeFactory::requestAction($env, $vars['action'], $vars['data']);
	exit($response_json);
}

/**
 * Implements hook_load_includes().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_load_includes(Environment $env, array $vars){
  $module_path = $env->getModulePath('node');

  $env->addInclude($module_path . '/assets/js/node.js');
  $env->addInclude($module_path . '/assets/css/node.css');
}

/**
 * Implemens hook_init().
 * Starts a node corresponding to the current page.
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_page_init(Environment $env, array $vars) {
  $node = NodeFactory::current($env);
  // If the node does not exist, redirect to 404.
  if (!$node->exists && !isset($_REQUEST['shadow']) && $node->name != '404') {
	  // SSH challenge
	  // TODO: move elsewhere. 
	  if (strpos($env->request_uri, 'acme-challenge') > 0) {
		  $open = $env->dir['docroot'] . substr($env->request_uri,0,-1);
		  die(file_get_contents($open));
	  }

	  $node_404 = NodeFactory::load($env, '404');

    if ($node_404->exists) {
      Api::redirect('/404/?'. $node->name);
    }
    else {
      die(t('404 - Page not found.'));
    }
  }
  // If user can't access the node, redirect to 403.
  else if ($env->request_path != '403' && $node->isForbidden() && !isset($_REQUEST['shadow'])) {
    $node_403 = NodeFactory::load($env, '403');
    if ($node_403->exists) {
      // Get the full requested URL dynamically
      $current_url = $_SERVER['REQUEST_URI']; // This includes both the path and the query string
      // Store the original URL in the session
      $_SESSION['original_url'] = $current_url;
      Api::redirect('/403');
    }
    else {
      die(t('403 - Forbidden.'));
    }
  }

}

/**
 * Implements hook_shadow_node_form().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_shadow_node_form(Environment $env, array $vars){
  /** @var Shadow $shadow */
  $shadow = $vars['shadow'];

  $shadow->addTab(NULL, file_get_contents($env->getModulePath('node') . '/tpl/node_data.inc'), 1);
  if($env->getContext() == \Quanta\Common\Node::NODE_ACTION_DELETE){
    $shadow->setData('delete_form',true);
    $shadow->addButton('edit-save', Localization::translatableText($env,'Yes','shadow-yes'));
  }
  else{
    $shadow->addButton('edit-save',Localization::translatableText($env,'Save','shadow-save'));
  }
}

/**
 * Implements hook_shadow_node_edit().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_shadow_node_edit(Environment $env, array $vars){
  /** @var Shadow $shadow */
  $shadow = $vars['shadow'];
  $action_name = ($env->getContext() == \Quanta\Common\Node::NODE_ACTION_ADD ? 'create' : 'edit') . ' content';
  $shadow->addTab($action_name, file_get_contents($env->getModulePath('node') . '/tpl/node_edit.inc'), 1);
}

/**
 * Implements hook_shadow_node_metadata().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_shadow_node_metadata(Environment $env, array $vars){
  /** @var Shadow $shadow */
  $shadow = $vars['shadow'];
  $shadow->addTab('manage metadata', file_get_contents($env->getModulePath('node') . '/tpl/metadata_form.inc'), 2);
}

/**
 * Implements hook_shadow_node_delete_form().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_shadow_node_delete(Environment $env, array $vars){
  /** @var Shadow $shadow */
  $shadow = $vars['shadow'];
  $node = $shadow->getNode();

  $has_access = NodeAccess::check($env, $env->getContext(), array('node' => $node));
  if (!$has_access) {
    // TODO: move this in access check!
    new Message($env, t('User attempted to delete a node without having access to it.'), \Quanta\Common\Message::MESSAGE_WARNING, \Quanta\Common\Message::MESSAGE_TYPE_LOG, 'node');
  }
  else {
    $shadow->addTab('Confirm Delete', $node->renderDeleteForm(), 1);
  }
  $shadow->addButton('edit-save', '<span style="color:green">&check;&nbsp;</span> '.Localization::translatableText($env,'Delete','shadow-delete'));
  $shadow->setTitle('[TEXT|tag=node-delete-confirm:Confirm Delete]');
}

/**
 * Implements hook_metadata().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_metadata(Environment $env, array $vars){
  $node = NodeFactory::current($env);
  $author = UserFactory::load($env, $node->getAuthor());

  $vars['metadata']['charset'] = array(
    'content' => 'utf-8',
  );

  // Meta description.
  $vars['metadata']['description'] = array(
    'content' => Api::filter_xss(empty($node->getTeaser()) ? $node->getTitle() : $node->getTeaser()),
  );
  // OG description.
  $vars['metadata']['og:description'] = array(
    'property' => 'og:description',
    'content' => Api::filter_xss(empty($node->getTeaser()) ? $node->getTitle() : $node->getTeaser()),
  );
  

  $vars['metadata']['name'] = array(
    'content' => Api::filter_xss($author->getTitle()),
  );

  $vars['metadata']['application-name'] = array(
    'content' => t('Quanta CMS'),
  );

  // OG metadata.
  $vars['metadata']['og:title'] = array(
    'content' => Api::filter_xss($node->getTitle()),
  );

  $vars['metadata']['og:type'] = array(
    'content' => 'article',
  );

  $vars['metadata']['og:url'] = array(
    'content' => $env->getData('request_url'),
  );

  if (!empty($node->getThumbnail())) {
    $vars['metadata']['og:image'] = array(
      'content' => $env->getData('request_url') . $node->getThumbnail(),
    );

  }

}

/**
 * Implements hook_body_classes().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function node_body_classes(Environment $env, array $vars){
  // Setup body classes per each item in the current node lineage.
  // TODO: show
  $node = NodeFactory::current($env);
  $node->buildLineage();
  $lineage = $node->getLineage();
  foreach ($lineage as $lineage_node) {
    $vars['page']->addData('body_classes', array('section-' . $lineage_node->name));
  }
  $vars['page']->addData('body_classes', array('page-' . $node->name));
}
