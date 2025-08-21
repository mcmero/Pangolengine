<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.11.0" name="objects" tilewidth="160" tileheight="180" tilecount="3" columns="0" objectalignment="topleft">
 <grid orientation="orthogonal" width="1" height="1"/>
 <tile id="2" type="player">
  <properties>
   <property name="animations" value="../info/player_anims.json"/>
   <property name="sprite_offset_x" type="float" value="8"/>
   <property name="sprite_offset_y" type="float" value="0"/>
   <property name="spritesheet" value="../textures/player_anim.png"/>
  </properties>
  <image source="../textures/player.png" width="32" height="32"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="11.5313" y="14.0625" width="9.0625" height="15.5313"/>
  </objectgroup>
 </tile>
 <tile id="0">
  <image source="../textures/npc.png" width="32" height="32"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="11.5036" y="12.9846" width="9.05823" height="17.0832"/>
  </objectgroup>
 </tile>
 <tile id="1">
  <image source="../textures/SpritesheetBuilding.png" width="160" height="180"/>
 </tile>
</tileset>
