<?php
	require_once( dirname(__FILE__)."/repix_core.inc" );

	$path=$_SERVER[ 'PHP_SELF' ];
	$base=dirname($_SERVER['SCRIPT_NAME'])."/".basename(__FILE__);
	$path=substr( $path , strlen($base) );


	$repix=new Repix( $_REQUEST );
	$repix->resize( $path , null );

?>
