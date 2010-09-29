<?php
/**
 * @file
 * Saffronvideo utilities.
 */


class SaffronVideoMgr {

  // form fields
  const FIELD_VIDEO_ASSET_ID = 'field_saffronvideo_url';
  const FIELD_VIDEO_AUTOLOAD_THUMBNAIL_IMG = 'field_saffronvideo_autoload_thumb_img';

  // variables
  const VAR_URL = 'saffronvideo_webservice';
  const VAR_PREROLL_URL = 'saffronvideo_preroll';  
  const VAR_USE_PROXY = 'saffronvideo_use_proxy';
  const VAR_PROXY_HOST = 'saffronvideo_proxy_host';
  const VAR_PROXY_PORT = 'saffronvideo_proxy_port';
  const VAR_CONTENT_TYPE_NAME = 'saffronvideo_content_type';
  const VAR_FIELD_VIDEO_ASSET_ID = 'saffronvideo_field_video_asset_id_ref';
  const VAR_FIELD_PREVIEW_IMAGE_REF = 'saffronvideo_field_preview_img_ref';
  const VAR_AUTO_ADD_VIDEO_THUMBNAIL_IMG_AS_PREVIEW_IMG = 'saffronvideo_auto_add_video_preview_img';

  // variable defaults
  private static $variable_defaults = array(
    self::VAR_URL => 'unknown',
    self::VAR_PREROLL_URL => 'unknown',
    self::VAR_USE_PROXY => 0,
    self::VAR_PROXY_HOST => '64.210.197.20',
    self::VAR_PROXY_PORT => '80',
    self::VAR_CONTENT_TYPE_NAME => 'video',
    self::VAR_FIELD_VIDEO_ASSET_ID => 'field_saffron_asset_id',
    self::VAR_FIELD_PREVIEW_IMAGE_REF => 'field_preview_image',
    self::VAR_AUTO_ADD_VIDEO_THUMBNAIL_IMG_AS_PREVIEW_IMG => 1,
  );

  const PROTOCOL_HTTP = 'http';
  const PROTOCOL_RTMP = 'rtmp';
  const PROTOCOL_RTMPE = 'rtmpe';

  private static $supported_protocols = array(
    'href' => self::PROTOCOL_HTTP,
    'rtmp' => self::PROTOCOL_RTMP,
    'rtmpe' => self::PROTOCOL_RTMPE,
  );


  public static function get_webservice_hostname() {
    $parts = parse_url(self::get_var(self::VAR_URL));
    return $parts['host'];
  }


  /**
   * Get a variable.
   * @static
   * @return value of variable.
   */
  public static function get_var($name) {
    return variable_get($name, self::$variable_defaults[$name]);
  }


  /**
   * Get list of all available videos.
   *
   * @return array of (unique_id => SaffronVideo) pairs.
   */
  public static function get_all_videos() {
    $cached_videos = cache_get('saffronvideo_catalogue', 'cache_content');
    if (!$cached_videos) {
      $cached_videos = self::_get_videos_from_remote_server();
      cache_set('saffronvideo_catalogue', $cached_videos, 'cache_content', CACHE_TEMPORARY);
    } else {
      $cached_videos = $cached_videos->data;
    }
    return $cached_videos;
  }



  /**
   * Get all available videos which are not already associated with nodes.
   *
   * @param $dont_exclude Array of videos to include in the returned array (as specified by their asset IDs) even if they're already associated with nodes.
   *
   * @return array of SaffronVideo items
   */
  public static function get_unused_videos($dont_exclude = array()) {
    static $unused_videos = array();

    $dont_exclude_key = !empty($dont_exclude) ? implode(',',$dont_exclude) : NULL;
    if (!isset($unused_videos[$dont_exclude_key])) {

      $used_videos = array();
      $db_field_name = self::get_var(self::VAR_FIELD_VIDEO_ASSET_ID).'_value';
      $db_table_name = 'content_type_'.self::get_var(self::VAR_CONTENT_TYPE_NAME);
      $r = db_query("SELECT $db_field_name AS 'asset_id' FROM {$db_table_name} WHERE $db_field_name IS NOT NULL");
      while ($obj = db_fetch_object($r)) {
        $used_videos[] = $obj->asset_id;
      }

      $all_videos = self::get_all_videos();

      $unused_videos[$dont_exclude_key] = array();
      foreach ($all_videos as $v) {
        // add to list if it's either
        //  (1) in the dont-exclude list, OR
        //  (2) it isn't yet associated with a node
        if (in_array($v->asset_id, $dont_exclude) || !in_array($v->asset_id, $used_videos))
          $unused_videos[$dont_exclude_key][] = $v;
      }
    }

    return $unused_videos[$dont_exclude_key];
  }



  /**
   * Get the specified video
   *
   * @param $asset_id The asset id of the video.
   *
   * @return the SaffronVideo item if found; FALSE otherwise.
   */
  public static function get_video($asset_id) {
    $all_videos = self::get_all_videos();
    $asset_id_md5 = md5($asset_id);
    return isset($all_videos[$asset_id_md5]) ? $all_videos[$asset_id_md5] : FALSE;
  }



  private static function _get_videos_from_remote_server() {

    $url = self::get_var(self::VAR_URL);
    if (1 == self::get_var(self::VAR_USE_PROXY)) {
      $catalog = @simplexml_load_string(self::_load_url_via_proxy($url));
    } else {
      $catalog = @simplexml_load_file($url);
    }

    if (!$catalog) {
      self::log_error("Error loading XML catalog from $url");
      return array();
    }

    $videos = array();
    foreach ($catalog->item as $id => $item) {
      // id
      $v = new SaffronVideo();
      $v->asset_id = (string)$item["id"];
      // basic attributes
      $video_tag = $item->videos->video;
      $v->width = (string)$video_tag["width"];
      $v->height = (string)$video_tag["height"];
      // video urls
      $video_tag_attrs = $video_tag->attributes();
      $supported_protos = array_keys(self::$supported_protocols);
      foreach ($video_tag_attrs as $proto => $video_url) {
        if (in_array($proto, $supported_protos)) {
          $v->videos[self::$supported_protocols[$proto]] = array(SaffronVideo::URL => (string)$video_url, SaffronVideo::FILENAME => basename($video_url));
        }
      }
      // thumbnail images
      if (isset($item->images)) {
        // parse
        $images_parsed = array();
        foreach ($item->images->image as $image){
          $image_width = intval((string)$image['width']);
          $image_height = intval((string)$image['height']);
          $image_size = $image_width * $image_height;
          $images_parsed[$image_size] = array(
            SaffronVideo::URL => (string)$image['href'],
            SaffronVideo::WIDTH => $image_width,
            SaffronVideo::HEIGHT => $image_height,
            SaffronVideo::BYTES => intval((string)$image['size']),
          );
        }
        // sort
        krsort($images_parsed);
        // save
        $v->images = array_values($images_parsed);
      }
      // save
      $videos[md5($v->asset_id)] = $v;
    }

    return $videos;
  }


  /**
   * Load a URL via the proxy server.
   * @param  $url
   * @return response as string
   */
  private static function _load_url_via_proxy($url) {
    $proxy_fp = fsockopen(self::get_var(self::VAR_PROXY_HOST), self::get_var(self::VAR_PROXY_PORT));
    if (!$proxy_fp) {
      return false;
    }
    fputs($proxy_fp, "GET $url HTTP/1.0\r\nHost: " . self::get_var(self::VAR_PROXY_HOST) . "\r\n\r\n");
    while (!feof($proxy_fp)) {
      $proxy_cont .= fread($proxy_fp, 4096);
    }
    fclose($proxy_fp);
    $proxy_cont = substr($proxy_cont, strpos($proxy_cont, "\r\n\r\n") + 4);
    return $proxy_cont;
  }


  public static function log($msg) {
    self::_log($msg, WATCHDOG_INFO);
  }

  public static function log_error($msg) {
    self::_log($msg, WATCHDOG_ERROR);
  }

  private static function _log($msg, $severity) {
    watchdog('saffronvideo', $msg, array(), $severity);
  }


  /**
   * Return the JS object required for omniture to track the video stats, per
   * Custom Video Specs Implementation.doc supplied by Omniture.
   */
  public static function construct_omniture_props_js($node) {
    $obj = new stdClass();
    $obj->prop49 = addslashes($node->title);
    $obj->prop4 = addslashes($node->sub_title);
    $obj->prop10 = '';
    return drupal_to_js($obj);
  }

}


/**
 * Represents a video item.
 */
class SaffronVideo {

  const URL = 'url';
  const FILENAME = 'filename';
  const WIDTH = 'width';
  const HEIGHT = 'height';
  const BYTES = 'bytes';

  /** Unqiue id of video */
  public $asset_id = NULL;
  /** width of video */
  public $width = -1;
  /** height of video */
  public $height = -1;
  /** Array of (Protocol => array('url' => URL, 'filename' => filename))*/
  public $videos = array();
  /** Array of array('url' => URL, 'width' => width, 'height' => height, 'bytes' => size in bytes) in descending order of image size */
  public $images = array();
}
