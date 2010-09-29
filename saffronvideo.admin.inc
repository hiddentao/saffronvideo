<?php
// $Id$
/**
 * @file
 * Administration page callbacks for the schedule module.
 */

require_once('saffronvideo.class.inc');


/**
 * Form builder. Configure schedule.
 *
 * @ingroup forms
 * @see system_settings_form().
 */
function saffronvideo_admin_settings() {

  $form['saffronserver'] = array(
    '#type' => 'fieldset', 
    '#title' => t('Saffron Server') 
  ); 
  
  $form['saffronserver'][SaffronVideoMgr::VAR_URL] = array(
    '#type' => 'textfield', 
    '#title' => t('Webservice URL'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_URL),
    '#description' => t('The URL to saffronvideo information.')
  ); 

  $form['saffronserver'][SaffronVideoMgr::VAR_PREROLL_URL] = array(
    '#type' => 'textfield',
    '#title' => t('Pre-roll URL'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_PREROLL_URL),
    '#description' => t('The URL to the pre-roll.')
  );
  
  $form['saffronserver']['proxy'] = array(
    '#type' => 'fieldset',
    '#title' => t('Proxy configuration')
  );

  $form['saffronserver']['proxy'][SaffronVideoMgr::VAR_USE_PROXY] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Proxy'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_USE_PROXY),
    '#description' => t('Whether to use a proxy server or not.')
  );

  $form['saffronserver']['proxy'][SaffronVideoMgr::VAR_PROXY_HOST] = array(
    '#type' => 'textfield',
    '#title' => t('Hostname'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_PROXY_HOST),
    '#description' => t('The proxy server address.')
  );

  $form['saffronserver']['proxy'][SaffronVideoMgr::VAR_PROXY_PORT] = array(
    '#type' => 'textfield',
    '#title' => t('Port'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_PROXY_PORT),
    '#description' => t('The proxy server port.')
  );

  // content type stuff
  $form['cck'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content type configuration')
  );
  $form['cck'][SaffronVideoMgr::VAR_CONTENT_TYPE_NAME] = array(
    '#type' => 'textfield',
    '#title' => t('Content type name'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_CONTENT_TYPE_NAME),
    '#desription' => t('The content type with which to associate videos'),
  );
  $form['cck'][SaffronVideoMgr::VAR_FIELD_VIDEO_ASSET_ID] = array(
    '#type' => 'textfield',
    '#title' => t('Field which holds the Saffron asset ID'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_FIELD_VIDEO_ASSET_ID),
    '#desription' => t('The name of the field which stores the Saffron video asset ID. Must be a text field.'),
  );
  $form['cck']['image'] = array(
    '#type' => 'fieldset',
    '#title' => 'Video preview image',
  );
  $form['cck']['image'][SaffronVideoMgr::VAR_FIELD_PREVIEW_IMAGE_REF] = array(
    '#type' => 'textfield',
    '#title' => t('Field which holds the video preview image'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_FIELD_PREVIEW_IMAGE_REF),
    '#desription' => t('The name of the field in the content type which stores the video preview image. Must be a file field.'),
  );
  $form['cck']['image'][SaffronVideoMgr::VAR_AUTO_ADD_VIDEO_THUMBNAIL_IMG_AS_PREVIEW_IMG] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically set the video thumbnail image as the preview image for the node'),
    '#default_value' => SaffronVideoMgr::get_var(SaffronVideoMgr::VAR_AUTO_ADD_VIDEO_THUMBNAIL_IMG_AS_PREVIEW_IMG),
    '#desription' => t('If checked then the largest image thumbnail available for a video from the Saffron server will be set as the preview image for a node.'),
  );

  return system_settings_form($form); 
} 


