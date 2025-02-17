<?php
namespace Quanta\Common;

/**
 * Implements hook_shadow_FORMID_form().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function file_shadow_file_form(Environment $env, array $vars) {
  /** @var Shadow $shadow */
  $shadow = $vars['shadow'];
  $node = $shadow->getNode();
  if (NodeAccess::check($env, $env->getContext(), array('node' => $node))) {
    $vars['shadow']->addTab('manage files', file_get_contents($env->getModulePath('file') . '/tpl/upload_form.html'), 2);
  }
}

function file_shadow_simple_file_form(Environment $env, array $vars) {
  /** @var Shadow $shadow */
  $shadow = $vars['shadow'];
  $node = $shadow->getNode();
  if (NodeAccess::check($env, $env->getContext(), array('node' => $node))) {
    $vars['shadow']->addTab('manage files', file_get_contents($env->getModulePath('file') . '/tpl/simple_upload_form.html'), 2);
    $vars['shadow']->setTitle('[RESOLUTION_MESSAGE|min={SHADOW:min_resolution}|max={SHADOW:max_resolution}]');
  }
}

/**
 * Implements hook_load_includes().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function file_load_includes(Environment $env, array $vars) {
  $module_path = $env->getModulePath('file');
  $env->addInclude($module_path . '/assets/css/file.css');
}

/**
 * Implements hook_init().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function file_init(Environment $env, array $vars) {
  // There is a request to upload a file.
  // TODO: move in actions / factory?
  if (!empty($_FILES)) {
    FileObject::checkUploads($env);
    exit();
  }

  // There is a request to delete a file.
  // TODO: move in actions / factory?
  if (isset($_REQUEST[\Quanta\Common\Node::NODE_ACTION_DELETE_FILE])) {
    $node = NodeFactory::current($env);
    $split_file_to_delete = explode('/', $_REQUEST[\Quanta\Common\Node::NODE_ACTION_DELETE_FILE]);
    FileObject::deleteFile($node, $split_file_to_delete[count($split_file_to_delete) - 1]);
    exit();
  }
}

/**
 * Implements hook_node_presave().
 *
 * Saves all files uploaded on the node.
 *
 * @param Environment $env
 *   The Environment.
 * @param $vars
 *   An array of variables.
 */
function file_node_presave(Environment $env, array $vars) {
  /** @var Node $node */
  $node = $vars['node'];
  $data = $vars['data'];
  $set_auto_thumbnail = isset($data['set_auto_thumbnail']) && $data['set_auto_thumbnail'] ? true : false;

  $uploaded_files = [];
  $isMultiple= !isset($data['single_file']) || (isset($data['single_file']) && empty($data['single_file']->value));
  $setAsThumbnail = !$isMultiple && $data['set_as_thumbnail']->value;
  // Walk through uploaded files.
  foreach ($data as $label => $name) {
    $exp = explode('-', $label);
    if (array_shift($exp) == 'uploaded_file') {
      $form_name = array_shift($exp);
      $file = implode('-', $exp);
      $uploaded_files[$form_name][] = Api::normalizeFilePath($file);
    }
  }

  //
  foreach ($uploaded_files as $form_name => $uploaded_file_list) {
    // Save list of uploaded files for this file field in json.
    $node->json->{$form_name} = $isMultiple ? $uploaded_file_list : $uploaded_file_list[count($uploaded_file_list) - 1];
    if($setAsThumbnail){
      $node->setThumbnail($uploaded_file_list[count($uploaded_file_list) - 1]);
    }
  }
   // set thumbnail in case multiple files and no thumbnail
   if ($set_auto_thumbnail && $isMultiple && empty($node->json->thumbnail) && !empty($uploaded_file_list[0])) {
    $node->setThumbnail($uploaded_file_list[0]);
  }
}

/**
 * Implements hook_node_save().
 *
 * Saves all files uploaded on the node.
 *
 * @param Environment $env
 *   The Environment.
 * @param $vars
 *   An array of variables.
 */
function file_node_after_save(Environment $env, array &$vars) {
  // TODO: move in File or create FileFactory module.
  /** @var Node $node */
  $node = $vars['node'];
  $uploaded_files = array();
	if (!empty($node->getData('tmp_files_dir'))) {
  $tmp_dir = $env->dir['tmp_files'] . '/' . $node->getData('tmp_files_dir');
  if (is_dir($tmp_dir)) {
    $tmp_files = $env->scanDirectory($tmp_dir);
    foreach ($tmp_files as $file) {
      $tmp_file_path = $tmp_dir . '/' . $file;

      $fileobj = new FileObject($env, $tmp_file_path, $node);
      // Check that uploaded files are valid.
      if (!$fileobj->isPublic()) {
        new Message($env, t('The file <b>!file</b> has an invalid name and could not be uploaded. Please use a different name.', array('!file' => $file)), \Quanta\Common\Message::MESSAGE_WARNING);
      }
      elseif (is_file($tmp_file_path)) {
        $normalize_file_path = Api::normalizeFilePath($file);
        copy($tmp_file_path, $node->path . '/' . $normalize_file_path);
        $uploaded_files []= $normalize_file_path;
        unlink($tmp_file_path);
        
      }
      else {
        new Message($env, t('The file <b>!file</b> is invalid and could not be uploaded.', array('!file' => $file)), \Quanta\Common\Message::MESSAGE_WARNING);
      }
    }
    $vars['uploaded_files'] = $uploaded_files;
    rmdir($tmp_dir);
  }
  }
}

/**
 * Implements hook_page_init().
 *
 * @param Environment $env
 *   The Environment.
 * @param array $vars
 *   An array of variables.
 */
function file_page_init(Environment $env, array $vars) {
  // If a Shadow is open, initialize file upload libraries.
	if (Shadow::isOpen($env)) {
    /** @var Page $page */
    $page = $vars['page'];
    // TODO: print is not a good approach.
    $attributes = array('file_inline' => TRUE, 'refresh' => TRUE, 'module' => 'file');
    print new \Quanta\Qtags\Js($env, $attributes, '/assets/js/jquery.knob.js');
    print new \Quanta\Qtags\Js($env, $attributes, '/assets/js/jquery.iframe-transport.js');
    print new \Quanta\Qtags\Js($env, $attributes, '/assets/js/jquery.fileupload.js');
    print new \Quanta\Qtags\Js($env, $attributes, '/assets/js/file-upload.js');
  }
}


function file_shadow_preview_img($env, $vars) {
	$vars['shadow']->addTab('Preview', file_get_contents($env->getModulePath('file') . '/tpl/preview_img.inc'), 2);
	$vars['shadow']->addButton('set_as_thumbnail', t('Imposta come principale'));
	$vars['shadow']->addButton('delete_img', t('Elimina'));
  $vars['shadow']->setTitle('[TEXT|tag=file-operations:Operazioni sulle immagini]');
}

function file_file_operations_form_after_submit($env, $vars){
	$response = new \stdClass();
	$success = true;
	$errors = array();
	http_response_code(200);
  $form_state = $vars['form_state'];
	$action = $form_state->getData('action_type');
	$node_name = $form_state->getData('img_node');
	$img_name = $form_state->getData('img');
	$img_key = $form_state->getData('img_key');
	$current_url = $form_state->getData('current_url');
	$node = \Quanta\Common\NodeFactory::load($env,$node_name);
	$img_path = $node->getPath() . '/' . $img_name;
	$node_files = !empty($node->getAttributeJSON('files')) ? $node->getAttributeJSON('files') : array();
	$node_files = !is_array($node_files) ? array($node_files) : $node_files;

	switch ($action) {
		case 'set_as_thumbnail':
				$node->setThumbnail($img_name);
				$node->setLanguage(\Quanta\Common\Localization::LANGUAGE_NEUTRAL);
				$node->save();
			break;

		case 'delete_img':
			if (file_exists($img_path)) {
				// Attempt to delete the file
				if (unlink($img_path)) {
					if(in_array($img_name,$node_files)){
						  // Remove the image from the array
						$node_files = array_filter($node_files, function($file) use ($img_name) {
							return $file !== $img_name;
						});
						// Re-index the array to remove any gaps in the numeric keys
						$node_files = array_values($node_files);
						$node->setAttributeJSON('files', $node_files);
					}
					if($node->getThumbnail() == $img_name){
						$node->setThumbnail(null);
					}
          if(!empty($node->json->{$img_key})){
            $node->removeAttributeJSON($img_key);
          }
					$node->setLanguage(\Quanta\Common\Localization::LANGUAGE_NEUTRAL);
					$node->save();
				} else {
					$success = false;
					$errors['error-message'] = 'Failed to delete the image file from the server';
				}
			} else {
				$success = false;
				$errors['error-message'] = 'Image file does not exist on the server';
			}
			break;
		
		default:
			$success = false;
			$errors ['error-message']= 'Invalid action type';
			break;
	}
  $response->success = $success;
  $response->img = $img_name;
  $response->action_type = $action;
	if(!$success){
		http_response_code(400);
		$response->shadowErrors = json_encode($errors);
	}
	$response_json = json_encode($response);
	exit($response_json);
}
