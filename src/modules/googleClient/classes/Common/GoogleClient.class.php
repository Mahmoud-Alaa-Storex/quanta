<?php
namespace Quanta\Common;

use Google_Client; // Import the Google_Client class
use Google_Service_Oauth2; // Import the Google_Service_Oauth2 class
use Google_Service_Docs;

/**
 * Class GoogleClient
 */
class GoogleClient{

    const GENERATE_GOOGLE_DOC_PATH = "generate-google-doc";

    public $client = NULL;
    public $service = NULL;


    public function __construct(){
        // Set the Google API credentials
        $this->client = new Google_Client();
        $this->client->setClientId($env->getData('GOOGLE_CLIENT_ID'));
        $this->client->setClientSecret($env->getData('GOOGLE_CLIENT_SECRET'));
        $this->client->addScope(Google_Service_Docs::DOCUMENTS);
        $this->client->setRedirectUri($env->getData('GOOGLE_CLIENT_REDIRECT_URL'));
        $this->client->setAccessType('offline');
        $this->client->setPrompt('select_account consent');
        if(isset($_SESSION['google_access_token'])){
            $this->client->setAccessToken($_SESSION['google_access_token']);
            $this->refreshAccessToken();

        }
    }

   
    /**
     * Check if the google access token stored in the session
     */
    public function checkSession(){

        if (!isset($_SESSION['google_access_token']) || $_SESSION['google_access_token'] === null) {
            $authUrl = $this->client->createAuthUrl();
            \Quanta\Common\API::redirect(filter_var($authUrl, FILTER_SANITIZE_URL));
        }
    }


    /**
     * Set google_access_token in the session.
     * @param String $code
     */
    public function setAccessToken($code){
        // Exchange authorization code for an access token
        $this->client->authenticate($code);
        $_SESSION['google_access_token'] = $this->client->getAccessToken();
    }

    /**
     * Refresh the access token
     * @param String $code
     */
    public function refreshAccessToken(){
    // Refresh the token if it's expired
        if ($this->client->isAccessTokenExpired()) {
            $this->client->fetchAccessTokenWithRefreshToken($this->client->getRefreshToken());
            $_SESSION['access_token'] = $this->client->getAccessToken();
        }
    }

}
