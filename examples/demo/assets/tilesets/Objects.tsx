<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.10" tiledversion="1.11.0" name="objects" tilewidth="208" tileheight="240" tilecount="15" columns="0" objectalignment="topleft">
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
   <object id="1" x="11.5313" y="17.8713" width="9.0625" height="11.7225"/>
  </objectgroup>
 </tile>
 <tile id="0">
  <image source="../textures/npc.png" width="32" height="32"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="11.5036" y="14.4846" width="9.05823" height="15.5832"/>
  </objectgroup>
 </tile>
 <tile id="6">
  <image source="../textures/tree.png" width="64" height="96"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="28.125" y="64.875" width="11.125" height="30.625"/>
  </objectgroup>
 </tile>
 <tile id="14">
  <image source="../textures/clock_tower.png" width="96" height="240"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="10.6667" y="48.3333" width="74" height="186.667"/>
  </objectgroup>
 </tile>
 <tile id="7">
  <image source="../textures/tower_building_left_end.png" width="80" height="180"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="10.3333" y="13.3333" width="61.3333" height="161.333"/>
  </objectgroup>
 </tile>
 <tile id="10">
  <image source="../textures/tower_building_center.png" width="64" height="180"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="7.33333" y="18.3333" width="47.6667" height="155.333"/>
  </objectgroup>
 </tile>
 <tile id="11">
  <image source="../textures/tower_building_door.png" width="64" height="180"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="7.66667" y="17" width="49" height="126.333"/>
  </objectgroup>
 </tile>
 <tile id="12">
  <image source="../textures/tower_building_right_end.png" width="80" height="180"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="7" y="16" width="60" height="156.333"/>
  </objectgroup>
 </tile>
 <tile id="13">
  <image source="../textures/long_building.png" width="208" height="96"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="6.66667" y="8.33333" width="193" height="81.3333"/>
  </objectgroup>
 </tile>
 <tile id="15">
  <image source="../textures/desk_with_computer.png" width="48" height="32"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="5.63636" y="17.1818" width="37.9091" height="11.3636"/>
  </objectgroup>
 </tile>
 <tile id="16">
  <image source="../textures/table.png" width="96" height="48"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="3.45455" y="20" width="88.227" height="25.091"/>
  </objectgroup>
 </tile>
 <tile id="17">
  <image source="../textures/chair_back.png" width="16" height="32"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="2.19036" y="16.7533" width="12.0174" height="13.7342"/>
  </objectgroup>
 </tile>
 <tile id="18">
  <image source="../textures/chair_front.png" width="16" height="32"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="1" y="16.5652" width="14" height="14.6522"/>
  </objectgroup>
 </tile>
 <tile id="19">
  <image source="../textures/chair_left.png" width="16" height="32"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="1.13043" y="17.3043" width="13.8696" height="14.0435"/>
  </objectgroup>
 </tile>
 <tile id="20">
  <image source="../textures/bookshelf.png" width="48" height="48"/>
  <objectgroup draworder="index" id="2">
   <object id="1" x="2.90909" y="16.7273" width="42.1818" height="29.4545"/>
  </objectgroup>
 </tile>
</tileset>
