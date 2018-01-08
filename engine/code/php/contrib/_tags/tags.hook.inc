<?php
function tags_shadow_node_edit($vars) {
  $user = UserFactory::current($vars['env']);
  if (!$user->checkAccess(NodeFactory::current($env), $vars['env']->getContext())) {
    return FALSE;
  }
  $form = '
  <form id="tags" method="POST" enctype="multipart/form-data">
  <div id="tags">Add tags here</div></form>';

  $vars['shadow']->addTab('manage tags', '<div class="shadow-hint">Hint: after uploading files, use the FILES tag to include this list of files in content.</div><div id="edit-tags">' . $form . '</div>', 3);

}