<?php
namespace Quanta\Qtags;
/**
 * Renders the close button.
 */
class ShadowCloseButton extends Qtag {

  /**
   * Render the Qtag.
   *
   * @return string
   *   The rendered Qtag.
   */
  public function render() {
    $html = "<div class=\"close-shadow-container shadow-cancel\"> 
    <svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">
    <g id=\"x-close\">
    <path id=\"Icon\" d=\"M18 6L6 18M6 6L18 18\" stroke=\"#667085\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>
    </g>
    </svg>
    </div>";
   return $html;
  }
}