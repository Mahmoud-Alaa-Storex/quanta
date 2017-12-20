<?php
  // Include the Environment module.
  include_once('core/environment/environment.module');
  include_once('core/cache/cache.module');

  // Create a new Environment.
  $env = new Environment(NULL);

  // Check if the current request is a file rendering request.
  $env->checkFile();

  // Load the environment.
  $env->load();

  // Start the user session.
  $env->startSession();

  // Run all system modules.
  $env->runModules();

  // Run the boot hook.
  $env->hook('boot');

  // Check if there is any requested action.
  $env->checkActions();

  // Start page's standard index.html.
  $page = new Page($env, 'index.html');
  $env->setData('page', $page);

  // Run the init hook.
  $env->hook('init', array('page' => &$page));

  // Load page's included files (CSS / JS etc.)
  $page->loadIncludes();

  // Build the page's HTML code.
  $page->buildHTML();

  // TODO: determine when to run doctor.
  if (isset($_GET['doctor'])) {
    $doctor = new Doctor($env);
    $doctor->runAllTasks();
  }
  else {
    print $page->render();
  }

  // Run the complete hook.
  $env->hook('complete');

  // End the bootstrap.
  exit();
?>
