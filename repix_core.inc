<?php
//ini_set("display_errors", On);
//error_reporting(E_ALL);


/******************************************************************************

repix php edition.

Copyright 2011-2018 (c) Hiroshi Nagayama / ambass inc.

https://ambass.co.jp

Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
Software, and to permit persons to whom the Software is furnished to do so, subject
 to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

/*
	parameters.

	width		...	resize width
	height		...	resize height
	size		...	shurinked file size(bytes)
	color		...	background color(RRGGBB)
	alpha		...	alphablend for background color
	fontSize	...	fontsize of copyright text
	fontRatio	...	font size ratio in resized image.(ignore fontSize)
	copy		...	copyright text
	nocache		...	not use cache image.(true/false)
	antiAlias	...	nop.
	useShadow	...	text shadow(true/false)
	shadowColor	...	text shadow color. (RRGGBB)
	useBorder	...	text border(true/false)
	borderColor	...	text border color. (RRGGBB)
	prePrinting	...	draw copyright text before resizing.
	keepAspect	...	keep aspect
	textColor	...	color for text drawing
	fontColor	...	nop.
	align		...	text align (left/center/right)
	valign		...	text vertical align(top/middle/bottom)
	fontName	...	fontfile name(webfont required for '/fonts' directory.)
	matMode		...	1.. Mat the resize margin / 2.. no Mat
	mode		...	repix mode / (externalEEE|direct|local)
	encoding	...	utf8
*/

ini_set('memory_limit' , '512M' );
define( 'REPIX_INIFILE' , dirname( __FILE__ )."/conf/repix.conf" );

class Repix{
	const HTTP404=404;
	const HTTP200=200;
	const OFFSET=2;

	private $whitelists	= null;
	private $safe_path	= "/tmp";

	private $requestParam	= null;
	private $w		= null;
	private $h		= null;
	private $size		= null;
	private $color		= null;
	private $fontRatio	= null;
	private $copy 		= null;
	private $nocache	= null;
	private $antiAlias	= null;
	private $useShadow	= null;
	private $useBorder	= null;
	private $prePrinting	= null;
	private $keepAspect	= null;
	private $shadowColor	= null;
	private $textColor	= null;
	private $fontColor	= null;
	private $borderColor	= null;
	private $align		= null;
	private $valign		= null;
	private $fontName	= null;
	private $fontSize	= null;
	private $matMode	= null;
	private $mode		= null;
	private $alpha		= null;
	private $encoding	= null;

	private function stampCopyright( $image , $copy , $fontSize ){
		$fn=dirname( __FILE__ ).'/fonts/'.$this->fontName ;
		$ag=explode("\n" , $copy );
		$ih=imagesy($image);
		$ix=imagesx($image);

		$len=0;

		$lst=array();
		$lenh=0;

		$bbox = imagettfbbox( $fontSize , 0 , $fn , $copy );
		$fh=abs( $bbox[5]-$bbox[1] );

		for( $i=0 ;$i<count($ag) ; $i++ ){
			$tmp=$ag[$i];
			$sb="";
			$xx=0;
			$c=preg_split( "//u", $tmp , -1 , PREG_SPLIT_NO_EMPTY );	
			for( $j=0 ; $j<count($c) ; $j++ ){
				$bbox = imagettfbbox( $fontSize , 0 , $fn , $sb.$c[$j] );
				$xx= abs($bbox[2]-$bbox[6] );
				if( $xx+1 >$ix ){
					if( strlen($sb)>0 ){
						$bbox = imagettfbbox( $fontSize , 0 , $fn , $sb );
						$hh=( abs($bbox[1]-$bbox[7] ) );
						$lenh+=$hh;
						array_push($lst,$sb );
						$sb="";
					}
					$sb.=$c[$j];
					$xx=0;
				}else{
					$sb.=$c[$j];
				}
			}
			if( strlen($sb)>0 ){
				$bbox = imagettfbbox( $fontSize , 0 , $fn , $sb );
				$hh=( abs($bbox[1]-$bbox[7] ));
				$lenh+=$hh;
				array_push($lst,$sb );
			}
		}



		imagelayereffect($image , IMG_EFFECT_OVERLAY );
		imagealphablending($image, true );
		imagesavealpha($image, true );


		$y= $ih - $lenh - Repix::OFFSET ;


		$fh=$lenh/count($lst);
		if( $this->valign==='top' ){
			$y=$fh;
		}else if ($this->valign=='middle' ){
			$y= ($ih-$lenh)/2 + $fh;
		}else {
			$y=$ih-$lenh-Repix::OFFSET*4+$fh;
		}

		$col=imagecolorallocatealpha($image , hexdec($this->textColor[0]) ,hexdec($this->textColor[1]) , hexdec($this->textColor[2] ) , 0);
		$scol=imagecolorallocatealpha($image , hexdec($this->shadowColor[0]) ,hexdec($this->shadowColor[1]) , hexdec($this->shadowColor[2] ) , 0);
		$bcol=imagecolorallocatealpha($image , hexdec($this->borderColor[0]) ,hexdec($this->borderColor[1]) , hexdec($this->borderColor[2] ) , 0);
		for( $j=0; $j<count($lst);$j++){

			$left=0;
			if( $this->align!=='left' ){
				$bbox = imagettfbbox( $fontSize , 0 , $fn , $lst[$j] );
				$wd=abs( $bbox[2]-$bbox[6] );
				if( $this->align==='right' ){
					$left=$ix-$wd-Repix::OFFSET*8;
				}else if( $this->align=='center' ){
					$left=($ix-$wd)/2;
				}
			}
			if( $this->useShadow ){
				for( $yy=0 ; $yy<3 ; $yy++ ){
					for( $xx=0 ; $xx<3 ; $xx++ ){
				imagettftext( $image , $fontSize , 0 , $left+1+$xx , $y+1+$yy , $scol , $fn , $lst[$j] );
					}
				}
			}

			if( $this->useBorder ){
				for( $yy=0 ; $yy<3 ; $yy++ ){
					for( $xx=0 ; $xx<3 ; $xx++ ){
						imagettftext( $image , $fontSize , 0 , $left+$xx-1 , $y+$yy-1 , $bcol , $fn , $lst[$j] );
					}
				}
			}

			imagettftext( $image , $fontSize , 0 , $left , $y , $col , $fn , $lst[$j] );
			$y+=$fh;
		}


	}
	private function getbool( $key ){
		$ret=false;
		if( isset( $this->requestParam[ $key ]  ) ){
			if( $this->requestParam[ $key ] =='true' ){
				$ret=true;
			}
		}
		return $ret;
	}
	private function getrgb( $key ){

		$varid="0123456789abcdef";
		$rgb=null;
		if( isset( $this->requestParam[ $key ]  ) ){
			$rgb=$this->requestParam[ $key ] ;
		}else{
			$rgb="000000";
		}
		if( strlen($rgb)<6 ){
			$rgb="000000";
		}

		for( $i=0 ; $i<strlen($rgb) ; $i++ ){
			if( strpos($varid, $rgb[$i])===FALSE ){
				$rgb="000000";
				break;
			}
		}
		$r=substr($rgb,0,2);
		$g=substr($rgb,2,2);
		$b=substr($rgb,4,2);

		return array( $r , $g , $b );
	}

	private function getparam( $key ){
		if( isset( $this->requestParam[ $key ]  ) ){
			return $this->requestParam[ $key ] ;
		}
		return null;
	}
	private function getSuffix( $ext ){
		$ret=$ext;
		if( $ret=='jpg' ){
				return 'jpeg';
		}
		return $ret;
	}

	function Repix( $requestParam ){


		$initParam=parse_ini_file( REPIX_INIFILE , true );

		$this->safe_path=$initParam["local"]["safe.dir"];
		$this->whitelists=$initParam["white_list"]["domain"];


		$this->requestParam=$requestParam;

		$this->w		= $this->getparam( "width" );
		$this->h		= $this->getparam( "height" );
		$this->size		= $this->getparam( "size" );
		$this->color		= $this->getrgb( "color" );
		$this->fontRatio	= $this->getparam( "fontRatio" );
		$this->copy 		= $this->getparam( "copy" );
		$this->nocache 		= $this->getbool( "nocache" );
		$this->antiAlias	= $this->getbool( "antiAlias" );
		$this->useShadow	= $this->getbool( "useShadow" );
		$this->useBorder	= $this->getbool( "useBorder" );
		$this->prePrinting	= $this->getbool( "prePrinting" );
		$this->keepAspect	= $this->getbool( "keepAspect" );
		$this->shadowColor	= $this->getrgb( "shadowColor" );
		$this->textColor	= $this->getrgb( "textColor" );
		$this->fontColor	= $this->getrgb( "fontColor" );
		$this->borderColor	= $this->getrgb( "borderColor" );
		$this->align		= $this->getparam( "align" );
		$this->valign		= $this->getparam( "valign" );
		$this->fontName		= $this->getparam( "fontName" );
		$this->fontSize		= $this->getparam( "fontSize" );
		$this->matMode		= $this->getparam( "matMode" );
		$this->mode		= $this->getparam( "mode" );
		$this->alpha		= $this->getparam( "alpha" );
		$this->encoding		= $this->getparam( "encoding" );


		if( $this->encoding==null || strlen($this->encoding)==0 ){
			$this->encoding='utf8';
		}
		if( $this->align==null || strlen($this->align)==0 ){
			$this->align='left';
		}
		if( $this->valign==null || strlen($this->valign)==0 ){
			$this->valign='bottom';
		}
		if( $this->fontName==null || strlen($this->fontName)==0 ){
			$this->fontName='ipag.ttf';
		}
		if( $this->mode==null || $this->mode!='local' 
			&& $this->mode!='external' && $this->mode!='direct'
		 ){
			$this->mode='local';
		}
		if( $this->copy!==null && strlen($this->copy)>0 ){
			$this->copy=mb_convert_encoding( $this->copy , "utf8" , $this->encoding  );
		}
		$this->alpha=intval( $this->alpha );
		if( $this->alpha > 127 ){
			$this->alpha=127;
		}

	}
	private function resizeCore( $image , $waspect , $haspect ){
		$cx=imagesx($image);
		$cy=imagesy($image);

		$w=$cx*$waspect;
		$h=$cy*$haspect;
		$newImage=imagecreatetruecolor( $w , $h );
		imagelayereffect($newImage , IMG_EFFECT_OVERLAY );
		imagealphablending($newImage, false );
		imagesavealpha($newImage, true );
		imagecopyresampled($newImage,  
                   $image,   
                   0,        
                   0,        
                   0,        
                   0,        
                   $w,   
                   $h,
                   $cx,
                   $cy 
                  );

		return $newImage;
	}

	private function is_correct_domain( $domain ){

		for( $i=0 ; $i<count($this->whitelists) ; $i++ ){
			if( strcmp($this->whitelists[$i] , $domain )===0 ){
				return true;
			}
		}
		return false;
	}

	public function get_resize_image( $sourceFile , $destFile=null ){

		try{
		$res=null;
		$image=null;
		$img2=null;
		$ext=pathinfo($sourceFile, PATHINFO_EXTENSION);
		$this->suffix=$this->getSuffix($ext);


		if( $this->mode==='external' ){

			$arr=explode( "/" , $sourceFile );
			if( count($arr)<4 || $arr[1]!=='http' && $arr[1]!=='https' ){
				return null;
			}
			if( $this->is_correct_domain( $arr[2] )===false ){
				return null;
			}
			$sourceFile=$arr[1]."://".$arr[2];
			for( $i=3; $i<count($arr) ; $i++ ){
				$sourceFile.="/".$arr[$i];
			}
			
		}else{
			if( $destFile===null ){
				$destFile=$this->makeDestName($sourceFile);
			}
			if( strpos($destFile , $this->safe_path )!==0 ||
				!file_exists( $sourceFile )
			){
				return null;
			}
		}
		$width=(isset($this->w))?intval($this->w):-1;
		$height=(isset($this->h))?intval($this->h):-1;
		$this->size=(isset($this->size))?intval($this->size):-1;
		if( $this->size===0 ){
			$this->size=-1;
		}


		$cx=$width;
		$cy=$height;


		if( $this->mode=='direct' || $this->mode=='external' ){
			$this->nocache=true;
		}else if( !file_exists( $destFile ) ){
			$this->nocache=true;
		}else if( filemtime($destFile)<filemtime($sourceFile ) ){
			$this->nocache=true;
		}
		if( $this->nocache ){

			if( $this->mode==='external' ){
				$image=file_get_contents( $sourceFile );
				if( $image===false ){
					return null;
				}
				$image=imagecreatefromstring( $image );
				if( $image===FALSE ){
					return null;
				}
				imagelayereffect($image , IMG_EFFECT_OVERLAY );
				imagealphablending($image, false );
				imagesavealpha($image, true );


				
			}else{
				$func="imagecreatefrom".$this->suffix;
				$image=$func($sourceFile);
			}


			$resize=-1;
			$resize_source=-1;
			$aspect=-1;
	
			$imgw=imagesx($image);
			$imgh=imagesy($image);
			if( $this->h==null && $height==-1 ){
				$height=$imgh;
			}
			if( $this->w==null && $width==-1 ){
				$width=$imgw;
			}
			if( $height>$imgh ){
				$height=$imgh;
			}
			if( $width>$imgw ){
				$width=$imgw;
			}
			$waspect=1;
			$haspect=1;
			if( $width!==0 ){
				$waspect=$imgw/$width;
			}
			if( $height!==0 ){
				$haspect=$imgh/$height;
			}

			//echo $imgw."/".$imgh."<br>";
			//echo $waspect."/".$haspect."<br>";

			if( $this->keepAspect && ( $this->w===null && $this->h!==null || $this->w!==null && $this->h===null ) ){
				if( $this->w!=null ){
					$aspect=($width/$imgw)	;
					$cy=round( $imgh*$aspect );
					if( $cx>$width ){
						$glow=($cx/$width);
						$cy=round( $cy*$glow );
					}
				}else if( $h!=null ){
					$aspect=($height/$imgh)	;
					$cx=round( $imgw*$aspect );
					if( $cy>$height ){
						$glow=($cy/$height);
						$cx=round( $cx*$glow );
					}
				}
			}else{
				if( $waspect>$haspect && $width!=-1 ){
					$resize=$width;
					$resize_source=$imgw;
				}else if( $height!=-1 ){
					$resize=$height;
					$resize_source=$imgh;
				}
				$aspect=$resize/$resize_source;
			}
			if( $cy==-1 ){
				$cy=$imgh;
			}
			if( $cx==-1 ){
				$cx=$imgw;
			}
			//echo "keepAspect width=".$width.":height=".$height." cx=".$cx.":cy=".$cy ;
	
			if( $height==-1 ){
				$height=imgh;
			}
			if( $width==-1 ){
				$width=imgw;
			}

			if( $this->fontRatio!=null && strlen($this->fontRatio)>0 ){
				$fr=floatval($this->fontRatio);
				if( $fr>1.0 ){ $fr=1.0; }
				if( $fr<=0.0 ){$fr=0.05;}
				$this->fontRatio=$fr;
				if( $this->prePrinting ){
					$this->fontSize=round( $imgw*$fr );
				}else{
					$this->fontSize=round( $cx*$fr );
				}
			}

			if( strlen(trim($this->copy))>0 && $this->prePrinting ){
				$this->stampCopyright( $image , $this->copy , $this->fontSize );
			}
			$resized=false;

			if( $imgw > $cx || $imgh > $cy ){
				$res=$this->resizeCore( $image , $aspect , $aspect );
				$resized=true;
			}else{
				$res=$image;
			}

			$nomat=false;
			switch( $this->matMode ){
				case '1':
					$nomat=(!$resized);
					break;
				case '2':
					$nomat=true;
					break;
			}
			if( $nomat ){
				$res=$this->resizeCore( $image , $aspect , $aspect );
				$resized=true;
			}else{

				$resw=imagesx($res);
				$resh=imagesy($res);

				$x=($cx-$resw)/2;
				$y=($cy-$resh)/2;

				$img2=imagecreatetruecolor( $cx , $cy );
				imagelayereffect($img2 , IMG_EFFECT_OVERLAY );
				imagealphablending($img2, false );
				imagesavealpha($img2, true );
				$col=imagecolorallocatealpha($img2 , hexdec($this->color[0]) ,hexdec($this->color[1]) , hexdec($this->color[2] ) , $this->alpha );
/*
echo $this->color[0]."<br>";
echo $this->color[1]."<br>";
echo $this->color[2]."<br>";
*/
		imagelayereffect($img2 , IMG_EFFECT_OVERLAY );
		imagealphablending($img2, true );
		imagesavealpha($img2, true );
				imagefill( $img2 , 0 , 0 , $col );

				imagecopy(
			  	$img2,  
				   $res,   
				   $x,        
				   $y,        
				   0,        
				   0,        
				   $resw,   
				   $resh
				  );
				$res=$img2;
			}

			if( strlen(trim($this->copy))>0 && $this->prePrinting===false ){
				$this->stampCopyright( $res , $this->copy , $this->fontSize );
			}

			if( $this->mode=='local' ){
				$complete=false;


				$quality=100;
				$sub=0;
				$goal=0;
				if( $this->suffix=='jpeg' ){
					$quality=100;
					$sub=-5;
					$goal=0;
				}else if( $this->suffix=='png' ){
					$quality=0;
					$sub=1;
					$goal=9;
				}else if( $this->suffix=='gif' ){
					$this->size=-1;
				}


				if( $this->size!=-1 ){
					do{
						if( $quality===$goal ){
							$complete=true;
						}
					
						$func='image'.$this->suffix;
						$func( $res , $destFile , $quality );
						clearstatcache( true , $destFile );
						$fs=filesize($destFile);
						if( $complete===false && $fs>$this->size ){
//echo $quality."/".$fs."/".$destFile."<br>";
							$quality+=$sub;
							continue;
						}
						$complete=true;
					}while( $complete===false );

				}else{
					$func='image'.$this->suffix;
					$func( $res , $destFile );
				}

				$func="imagecreatefrom".$this->suffix;
				$res=$func($destFile);
			}
		}else{
			// use cache.
			$func="imagecreatefrom".$this->suffix;
			$image=$func($destFile);
			$res=$image;
		}
		return $res;
		}catch( Exception $e ){
			return null;
		}
	}
	public function resize( $sourceFile , $destFile=null ){

		$ret=$this->get_resize_image( $sourceFile , $destFile );
		if( $ret===null ){
			header ('HTTP/1.0 404 Not Found');
			die('Not Found');
			return Repix::HTTP400;
		}

		header( 'Content-type: image/'.$this->suffix );
		$func='image'.$this->suffix;
		$func( $ret );

		return Repix::HTTP200;

	}
	private function makeDestName( $path ){
		$repixpath=$path;
		$suffix="_".$this->w."_".$this->h."_".$this->size;
		$idx=strrpos( $path , "." );
		$ext=substr($path,$idx);
		if( $idx>0 ){
			$repixpath=substr( $path , 0 , $idx ).$suffix.$ext;
		}
		return $repixpath;
	}
	
}

?>
