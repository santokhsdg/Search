<?php
require("DBConnection");
$sql1="SELECT * from trie where SONG=1;"
$sql2="SELECT * from trie where ARTIST=1 ;"
$sql3="SELECT * from trie where TYPE=1 ;"
$sql4="SELECT * from trie where ALBUM=1 ;"
$sql5="SELECT * from trie where BAND=1;"
$result1 = mysqli_query($con,$sql1); 
$result2 = mysqli_query($con,$sql2); 
$result3 = mysqli_query($con,$sql3); 
$result4 = mysqli_query($con,$sql4); 
$result5 = mysqli_query($con,$sql5); 


while($row=mysqli_fetch_assoc($result1))
{
   $word=$row["WORD"];  
   $pref=$row["PREF"]; 
   $res=exec("./pmm $word 0 $pref");       
}

while($row=mysqli_fetch_assoc($result2))
{
   $word=$row["WORD"]; 
   $pref=$row["PREF"]; 
   $res=exec("./pmm $word 1 $pref");       
}
while($row=mysqli_fetch_assoc($result3))
{
   $word=$row["WORD"];
   $pref=$row["PREF"]; 
   $res=exec("./pmm $word 2 $pref");       
}
while($row=mysqli_fetch_assoc($result4))
{
   $word=$row["WORD"]; 
   $pref=$row["PREF"];
   $res=exec("./pmm $word 3 $pref");       
}
while($row=mysqli_fetch_assoc($result5))
{
   $word=$row["WORD"];
   $pref=$row["PREF"]; 
   $res=exec("./pmm $word 4 $pref");       
}
mysqli_close($con);
?>
