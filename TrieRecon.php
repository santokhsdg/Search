<?php
require("DBConnection");
sql="SELECT * from trie;"
$result1 = mysqli_query($con,$sql); 
$num=mysqli_num_rows($result1);

$pathf="CORE/ReconFail.txt";
$filef = fopen($path,"r") or die("Unable to open file!");
fwrite($filef, ""); // empty file
fclose($filef);

$filef = fopen($path,"a") or die("Unable to open file!");

$i=0;
$done=0;
while($row=mysqli_fetch_assoc($result1))
{
    if($i==$num-1)
    {
        $done=1;
    }
   $word=$row["WORD"];
   $song=$row["SONG"];
   $artist=$row["ARTIST"];
   $album=$row["ALBUM"];
   $type=$row["TYPE"];
   $band=$row["BAND"];
   $pref=$row["PREF"]; 
   $res=exec("./rmm $done $word $song $artist $album $type $band $pref ");
    if($res!=0)
    {
       $txt=$word."\n";
       fwrite($filef, $txt);  
    }    
    $i++;
}
fclose($filef);


?>