
<?php 
session_start();

if(isset($_SESSION["song_check"]))
$_SESSION["song_check"]="null";

if(!isset($_SESSION["bool"]))
{    
    header("Location: index.php");
    exit();
}
if(isset($_SESSION["bool"]))
{    
    if($_SESSION["bool"]==0 || $_SESSION["bool"]=="")
    {
       header("Location: index.php");
        exit();
    }    
}
?>

<?php


function UpdateTrie($string,$flag)
{  /// add entry to trie table on updation
    include("DBConnection.php");
    $remove = " ";
    $res = explode($string, $remove);
    $num=count($res);
    for($i=0;$i<num;$i++)
    {
        
        $str=$res[$i];
        if($str!=""){
        $sql="SELECT * from trie where WORD='$str'";
        $ans=mysqli_query($con,$sql);
        $rows=mysqli_num_rows($ans)
         if($row>0)
        {
          $sql="UPDATE trie set $flag=1 where WORD='$str'";  
           mysqli_query($con,$sql);
        }
        else{
           $sql="INSERT INTO trie(WORD,$flag) values('$str',1)";  
           mysqli_query($con,$sql); 
        }
        
    }
        
    }
    
}
 
function UploadForm($song_id,$name,$artist1,$artist2,$artist3,$album,$genre,$composer,$publisher,$rlabel,$iswc,$isrc,$date,$pline,$song_path,$image_path,$radio,$band,$update,$size,$desc)
	{
        if($size>(1024*1024))
        {
            $size = $size/(1024*1024) ;
            $size= round($size, 2);
            $size = (string)($size);
            $size = $size." Mb";    
        }
        else if($size>1024)
        {
            $size = $size/(1024) ;
            $size= round($size, 2);
            $size = (string)($size);
            $size = $size." Kb";    
        }
        else
        {
            $size= round($size, 2);
            $size = (string)($size);
            $size = $size." B";
        }
		include("DBConnection.php");
      
        $res3=0;
		$res1 = mysqli_query($con,"INSERT INTO song(NAME,LOCATION,ARTIST1,ARTIST2,ARTIST3,GENRE,RELEASE_DATE,ALBUM_NAME,IMG_LOCATION,SIZE,SONG_ID,TYPE,UPDATED,DESCRIPTION) VALUES('$name','$song_path','$artist1','$artist2','$artist3','$genre','$date','$album','$image_path','$size','$song_id','$radio',$update,$desc)" );
       $res2 = mysqli_query($con,"INSERT INTO song_info(SONG_ID,COMPOSER,PUBLISHER,LABEL,ISWC,ISRC,PLINE,BAND) VALUES('$song_id','$composer','$publisher','$rlabel','$iswc','$isrc','$pline','$band')");
        
            $userid = $_SESSION["user"];
            $res3 = mysqli_query($con,"INSERT INTO uploads(USER_ID,SONG_ID) VALUES('$userid','$song_id')");
		
		if($res1==1 && $res2==1 && $res3==1)
        {
            UpdateTrie($name,"SONG");
            UpdateTrie($artist1,"ARTIST");
            UpdateTrie($artist2,"ARTIST");
            UpdateTrie($artist3,"ARTIST");
            UpdateTrie($album,"ALBUM");
            UpdateTrie($type,"TYPE");
            UpdateTrie($band,"BAND");
            return 1;
            
        }
        else
        {
            return 0;
        }  
	}



?>



<?php 
session_start();
    if($_SESSION['song_check']=="null")
{
$result="null";
$radio="";
$song_path="";
$image_path="";
        
if(isset($_POST['submit']))
{
    if(isset($_POST['optradio']))
{
$radio = $_POST['optradio'];  //  Displaying Selected Value
}
if(isset($_SESSION['song_path']))
{
$song_path =  $_SESSION['song_path'];
}
    if(isset($_SESSION['image_path']))
    {
    $image_path =  $_SESSION['image_path'];
    }
    $newpath = "PHP/Script/".$song_path ;
$objform = new InterfaceClass();
$date = $_POST['year']."-".$_POST['month']."-".$_POST['day'];
$sngid=uniqid('sng');
$update=0;
if(isset($_SESSION['admin'])){
    if($_SESSION['admin']==1)
    { // setting admin right of super user
       $update=1; 
    }
}
$x=UploadForm($sngid,trim($_POST['name']),trim($_POST['artist1']),trim($_POST['artist2']),trim($_POST['artist3']),trim($_POST['album']),
                          trim($_POST['genre']),trim( $_POST['composer']),
                          trim($_POST['publisher']), trim($_POST['rlabel']), trim($_POST['iswc']),trim($_POST['isrc']),$date,trim($_POST['pline'])                               ,$song_path,$image_path,$radio,trim($_POST['band']),$update,filesize($newpath)
                          ,trim($_POST['description'])) ;   

    $sngname=trim($_POST['name']); $sngname=strtolower($sngname);
    $art1=trim($_POST['artist1']); $art1=strtolower($art1);
    $art2=trim($_POST['artist2']); $art2=strtolower($art2);
    $art3=trim($_POST['artist3']); $art3=strtolower($art3);
    $album=trim($_POST['album']);  $album=strtolower($album);
    $band=trim($_POST['band']);    $band=strtolower($band);
    if($band==""){ $band="x??";}   $type=strtolower($type);
    if($art2==""){ $art2="x??";}
    if($art3==""){ $art3="x??";}
    if($album==""){ $album="x??";}
    $type=trim($radio);
    
    if($x==1)
    {
        if(isset($_SESSION['song_check']))
              {
             $_SESSION['song_check'] = "done";
              }
        
        if($update==1)
        {
          $output=exec("./umm $sngid , $sngname , $art1 $art2 $art3 , $album , $band , $type"); 
         // sngid ,song name,  artist , album , band, type
        }
         $result = "You Have Successfully uploaded Song to RollSide!";
        
        
    }
    else
    { 
        $result = "There was an error while uploading ...Please Try Again!";
    }
}
}
else
{
    $result = "Already Uploaded The Song.";
}
?> 


