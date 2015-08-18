// This is a subset of the skinning.js demo at: 
// http://huningxin.github.io/skinning_simd/index-asm.html
// It is modified to run on a single vertex of one mesh and print resulting heap on the console.
// Heap data is captured from the actual values in the workload.

function InitBuffer(buffer)
{
    HEAP32 = new Int32Array(buffer);
    HEAP32[0] = 12;
    HEAP32[1] = 60511200;
    HEAP32[2] = 221372;
    HEAP32[3] = 28;
    HEAP32[4] = 4;
    HEAP32[5] = 217852;
    HEAP32[6] = 110;
    HEAP32[7] = 0;
    HEAP32[8] = 108;
    HEAP32[9] = 1656;
    HEAP32[10] = 26604;
    HEAP32[11] = 3206;
    HEAP32[12] = 18216;
    HEAP32[13] = 206140;
    HEAP32[14] = 14;
    HEAP32[15] = 206364;
    HEAP32[16] = 22;
    HEAP32[17] = 18370;
    HEAP32[18] = 207596;
    HEAP32[19] = 33;
    HEAP32[20] = 208124;
    HEAP32[21] = 75;
    HEAP32[22] = 18733;
    HEAP32[23] = 212324;
    HEAP32[24] = 34;
    HEAP32[25] = 212868;
    HEAP32[26] = 89;
    HEAP32[27] = 1023066042;
    HEAP32[28] = 1058861809;
    HEAP32[29] = 0;
    HEAP32[30] = 2;
    HEAP32[31] = 1011798195;
    HEAP32[32] = 1059292430;
    HEAP32[33] = 2;
    HEAP32[34] = 3;
    HEAP32[35] = 1025346669;
    HEAP32[36] = 1059345714;
    HEAP32[37] = 5;
    HEAP32[38] = 2;
    HEAP32[39] = 1043068980;
    HEAP32[40] = 1057610229;
    HEAP32[41] = 7;
    HEAP32[42] = 4;
    HEAP32[43] = 1040722048;
    HEAP32[44] = 1057932334;
    HEAP32[45] = 11;
    HEAP32[46] = 1;
    HEAP32[47] = 1042359907;
    HEAP32[48] = 1058150052;
    HEAP32[49] = 12;
    HEAP32[50] = 4;
    //
    HEAP32[6646] = 1;
    HEAP32[6647] = 1041576680;
    HEAP32[6648] = 1048592778;
    HEAP32[6649] = 842;
    HEAP32[6650] = 1;
    HEAP32[6651] = 5;
    HEAP32[6652] = 1061831254;
    HEAP32[6653] = 1073310254;
    HEAP32[6654] = 1089420185;
    HEAP32[6655] = 1102950847;
    HEAP32[6656] = 0;
    HEAP32[6657] = -1097877147;
    HEAP32[6658] = 1059398163;
    HEAP32[6659] = 1060499752;
    HEAP32[6660] = 0;
    HEAP32[6661] = 1047672409;
    HEAP32[6662] = -1087870328;
    HEAP32[6663] = 1060572011;
    HEAP32[6664] = 0;
    HEAP32[6665] = 64;
    HEAP32[6666] = 1045886631;
    HEAP32[6667] = -1072936556;
    HEAP32[6668] = 1084139178;
    HEAP32[6669] = 1068827817;
    HEAP32[6670] = 0;
    HEAP32[6671] = -1088140882;
    HEAP32[6672] = 1054701738;
    HEAP32[6673] = 1059200743;
    HEAP32[6674] = 0;
    HEAP32[6675] = 1047826109;
    HEAP32[6676] = -1087918667;
    HEAP32[6677] = 1060603576;
    HEAP32[6678] = 0;
    HEAP32[6679] = 5;
    HEAP32[6680] = 1060320051;
    HEAP32[6681] = 1034528821;
    HEAP32[6682] = 1085119185;
    HEAP32[6683] = 1102589383;
    HEAP32[6684] = 0;
    HEAP32[6685] = -1130455544;
    HEAP32[6686] = 1049435132;
    HEAP32[6687] = 1064700183;
    HEAP32[6688] = 0;
    HEAP32[6689] = 1054089720;
    HEAP32[6690] = -1083987898;
    HEAP32[6691] = 1044795625;
    HEAP32[6692] = 0;
    HEAP32[6693] = 64;
    HEAP32[6694] = 1041865114;
    HEAP32[6695] = -1068328898;
    HEAP32[6696] = 1076148647;
    HEAP32[6697] = 1049797025;
    HEAP32[6698] = 0;
    HEAP32[6699] = -1095508626;
    HEAP32[6700] = 1039853112;
    HEAP32[6701] = 1064148841;
    HEAP32[6702] = 0;
    HEAP32[6703] = 1058776462;
    HEAP32[6704] = -1086305179;
    HEAP32[6705] = 1048813710;
    HEAP32[6706] = 0;
    HEAP32[6707] = 65;
    HEAP32[6708] = 1041865114;
    HEAP32[6709] = 1079154676;
    HEAP32[6710] = -1071335184;
    HEAP32[6711] = -1097687905;
    //
    HEAP32[54490] = 0;
    HEAP32[54491] = 1056964608;
    HEAP32[54492] = 1056964608;
    HEAP32[54493] = 1056964608;
    HEAP32[54494] = 1056964608;
    HEAP32[54495] = -1055993330;
    HEAP32[54496] = -1088223964;
    HEAP32[54497] = 1116638689;
    HEAP32[54498] = 0;
    HEAP32[54499] = -1091733141;
    HEAP32[54500] = -1090750259;
    HEAP32[54501] = -1090080954;
    HEAP32[54502] = -1090272296;
    HEAP32[54503] = -1050560442;
    HEAP32[54504] = -1091877674;
    HEAP32[54505] = 1118511748;
    HEAP32[54506] = 0;
    HEAP32[54507] = 1055750507;
    HEAP32[54508] = 1056733389;
    HEAP32[54509] = 1057402694;
    HEAP32[54510] = 1057211352;
    HEAP32[54511] = -1048359862;
    HEAP32[54512] = -1114615539;
    HEAP32[54513] = 1120902744;
    HEAP32[54514] = 0;
    HEAP32[54515] = -1091733141;
    HEAP32[54516] = -1090750259;
    HEAP32[54517] = -1090080954;
    HEAP32[54518] = -1090272296;
    HEAP32[54519] = -1054810835;
    HEAP32[54520] = 1096797902;
    HEAP32[54521] = 1120400608;
    HEAP32[54522] = 0;
    HEAP32[54523] = -1094149717;
    HEAP32[54524] = 1052693976;
    HEAP32[54525] = 1060407509;
    HEAP32[54526] = 1055572947;
    HEAP32[54527] = -1047451957;
    HEAP32[54528] = 1102544358;
    HEAP32[54529] = 1117589077;
    HEAP32[54530] = 0;
    HEAP32[54531] = -1091673479;
    HEAP32[54532] = -1088453552;
    HEAP32[54533] = -1102618381;
    HEAP32[54534] = -1088882766;
    HEAP32[54535] = -1048802976;
    HEAP32[54536] = 1103994490;
    HEAP32[54537] = 1116807292;
    HEAP32[54538] = 0;
    HEAP32[54539] = 1061020927;
    HEAP32[54540] = 1052738066;
    HEAP32[54541] = -1106614831;
    HEAP32[54542] = 1057635653;
    HEAP32[54543] = -1065181447;
    HEAP32[54544] = 1106899588;
    HEAP32[54545] = 1115071166;
    HEAP32[54546] = 0;
    HEAP32[54547] = -1085973758;
    HEAP32[54548] = 1044507239;
    HEAP32[54549] = 1025833482;
    HEAP32[54550] = -1088727335;
    HEAP32[54551] = 1081287590;
    HEAP32[54552] = 1107830763;
    HEAP32[54553] = 1114237975;
    HEAP32[54554] = 0;
    HEAP32[54555] = 1054641331;
    HEAP32[54556] = 1051924423;
    HEAP32[54557] = -1086112496;
    HEAP32[54558] = 1051339163;
    HEAP32[54559] = 1086327555;
    HEAP32[54560] = 1107371917;
    HEAP32[54561] = 1113020123;
}


function PrintOutputBuffer(buffer)
{
    HEAP32 = new Int32Array(buffer);
    for (var i = 15127800; i < 15127800 + 50; i++)
        WScript.Echo(HEAP32[i]); 
}

var buffer = new ArrayBuffer(64 * 1024 * 1024/*200 * 512 * 1024*/);
var MESH_MEMORY_SIZE = 302556;
var VERTEX_ELEMENTS = 11; // 3 Pos, 2 UV, 3 Norm, 3 Tangent
var VERTEX_STRIDE = 44;
var VERTEX_MEMORY_BASE = 60511200; // MESH_MEMORY_SIZE * 128
var VERTEX_NUM = 1737;
var VERTEX_MEMORY_SIZE = 76428; // VERTEX_NUM * VERTEX_ELEMENTS * 4

function _asmjsModuleSIMD (global, imp, buffer) {
    "use asm";
    var HEAPF32 = new global.Float32Array(buffer);
    var HEAP32 = new global.Int32Array(buffer);
    var HEAPU8 = new global.Uint8Array(buffer);
    var imul = global.Math.imul;
    var toF = global.Math.fround;
    var sqrt = global.Math.sqrt;
    var abs = global.Math.abs;
    var SIMD_Float32x4 = global.SIMD.Float32x4;
    var SIMD_Float32x4_load = SIMD_Float32x4.load;
    var SIMD_Float32x4_store = SIMD_Float32x4.store;
    var SIMD_Float32x4_mul = SIMD_Float32x4.mul;
    var SIMD_Float32x4_add = SIMD_Float32x4.add;
    var SIMD_Float32x4_sub = SIMD_Float32x4.sub;
    var SIMD_Float32x4_swizzle = SIMD_Float32x4.swizzle;
    var SIMD_Float32x4_splat = SIMD_Float32x4.splat;
    var VERTEX_ELEMENTS = 11; // 3 Pos, 2 UV, 3 Norm, 3 Tangent
    var VERTEX_STRIDE = 44;
    var f_VERTEX_POS_0_OFFSET = 0;
    var f_VERTEX_POS_1_OFFSET = 4;
    var f_VERTEX_POS_2_OFFSET = 8;
    var f_VERTEX_UV_0_OFFSET =  12;
    var f_VERTEX_UV_1_OFFSET = 16;
    var f_VERTEX_NORMAL_0_OFFSET = 20;
    var f_VERTEX_NORMAL_1_OFFSET = 24;
    var f_VERTEX_NORMAL_2_OFFSET = 28;
    var f_VERTEX_TANGENT_0_OFFSET = 32;
    var f_VERTEX_TANGENT_1_OFFSET = 36;
    var f_VERTEX_TANGENT_2_OFFSET = 40;
    
    // Memory Layout
    var HEAP_BASE = 0;
    // Header
    var HEADER_SIZE = 12;
    var i_MODEL_STRUCT_PTR_OFFSET = 0;
    var i_VERT_ARRAY_PTR_OFFSET = 4;
    var i_ANIMATION_STRUCT_PTR_OFFSET = 8;
    
    // Model struct
    var MODEL_STRUCT_SIZE = 16;
    var i_MODEL_MESHES_PTR_OFFSET = 0;
    var i_MODEL_MESHES_LENGTH_OFFSET = 4;
    var i_MODEL_JOINTS_PTR_OFFSET = 8;
    var i_MODEL_JOINTS_LENGTH_OFFSET = 12; 
    
    // Mesh struct
    var MESH_STRUCT_SIZE = 20;
    var i_MESH_VERT_OFFSET_OFFSET = 0;
    var i_MESH_VERTS_PTR_OFFSET = 4;
    var i_MESH_VERTS_LENGTH_OFFSET = 8;
    var i_MESH_WEIGHTS_PTR_OFFSET = 12;
    var i_MESH_WEIGHTS_LENGTH_OFFSET = 16;
    
    // Vert struct
    var VERT_STRUCT_SIZE = 16;
    var f_VERT_TEXCOORD_0_OFFSET = 0;
    var f_VERT_TEXCOORD_1_OFFSET = 4;
    var i_VERT_WEIGHT_INDEX_OFFSET = 8;
    var i_VERT_WEIGHT_COUNT_OFFSET = 12;
    
    // Weight struct
    var WEIGHT_STRUCT_SIZE = 56;
    var i_WEIGHT_JOINT_INDEX_OFFSET = 0;
    var f_WEIGHT_BIAS_OFFSET = 4;
    var f_WEIGHT_POS_0_OFFSET = 8;
    var f_WEIGHT_POS_1_OFFSET = 12;
    var f_WEIGHT_POS_2_OFFSET = 16;
    var f_WEIGHT_POS_3_OFFSET = 20;
    var f_WEIGHT_NORMAL_0_OFFSET = 24;
    var f_WEIGHT_NORMAL_1_OFFSET = 28;
    var f_WEIGHT_NORMAL_2_OFFSET = 32;
    var f_WEIGHT_NORMAL_3_OFFSET = 36;
    var f_WEIGHT_TANGENT_0_OFFSET = 40;
    var f_WEIGHT_TANGENT_1_OFFSET = 44;
    var f_WEIGHT_TANGENT_2_OFFSET = 48;
    var f_WEIGHT_TANGENT_3_OFFSET = 52;
    
    // Joint struct
    var JOINT_STRUCT_SIZE = 32;
    var f_JOINT_POS_0_OFFSET = 0;
    var f_JOINT_POS_1_OFFSET = 4;
    var f_JOINT_POS_2_OFFSET = 8;
    var f_JOINT_POS_3_OFFSET = 12;
    var f_JOINT_ORIENT_0_OFFSET = 16;
    var f_JOINT_ORIENT_1_OFFSET = 20;
    var f_JOINT_ORIENT_2_OFFSET = 24;
    var f_JOINT_ORIENT_3_OFFSET = 28;
    
    // Animation Struct
    var ANIMATION_STRUCT_SIZE = 24;
    var i_ANIMATION_HIERARCHY_PTR_OFFSET = 0;
    var i_ANIMATION_HIERARCHY_LENGTH_OFFSET = 4;
    var i_ANIMATION_BASEFRAME_PTR_OFFSET = 8;
    var i_ANIMATION_BASEFRAME_LENGTH_OFFSET = 12;
    var i_ANIMATION_FRAMES_PTR_OFFSET = 16;
    var i_ANIMATION_FRAMES_LENGTH_OFFSET = 20;
    
    // Hierarchy Struct
    var HIERARCHY_STRUCT_SIZE = 12;
    var i_HIERARCHY_PARENT_OFFSET = 0;
    var i_HIERARCHY_FLAGS_OFFSET = 4;
    var i_HIERARCHY_INDEX_OFFSET = 8;
    
    // BaseFrame Struct
    var BASEFRAME_STRUCT_SIZE = 32;
    var f_BASEFRAME_POS_0_OFFSET = 0;
    var f_BASEFRAME_POS_1_OFFSET = 4;
    var f_BASEFRAME_POS_2_OFFSET = 8;
    var f_BASEFRAME_POS_3_OFFSET = 12;
    var f_BASEFRAME_ORIENT_0_OFFSET = 16;
    var f_BASEFRAME_ORIENT_1_OFFSET = 20;
    var f_BASEFRAME_ORIENT_2_OFFSET = 24;
    var f_BASEFRAME_ORIENT_3_OFFSET = 28;
    
    // Frames Struct
    var FRAMES_STRUCT_SIZE = 8;
    var i_FRAMES_PTR_OFFSET = 0;
    var i_FRAMES_LENGTH_OFFSET = 4;
    
    // Frame
    var FRAME_STRUCT_SIZE = 4;
    var f_FRAME_VALUE_OFFSET = 0;

    var MESH_MEMORY_SIZE = 302556;
    
    function asmSkinSIMD(meshIndex) {
        meshIndex = meshIndex|0;

        var i = 0, j = 0, k = 0;
        var headerPtr = 0, modelPtr = 0,
            meshesPtr = 0, meshesLength = 0,
            jointsPtr = 0, jointsLength = 0,
            vertexArrayPtr = 0;

        var meshPtr = 0, vertsPtr = 0, vertsLength = 0,
            weightsPtr = 0, weightsLength = 0, vertPtr = 0, vertWeightsCount = 0,
            vertWeightsIndex = 0, weightPtr = 0, jointPtr = 0, vertexPtr = 0,
            jointIndex = 0, meshOffset = 0;
            
        var rotatedPos = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0), jointOrient = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0),
            weightPos = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0), ix4 = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0),
            jointPos = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0), weightBias = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0),
            vx4 = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0), weightNormal = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0),
            nx4 = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0), weightTangent = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0),
            tempx4 = SIMD_Float32x4(1.0, 1.0, 1.0, -1.0), tx4 = SIMD_Float32x4(0.0, 0.0, 0.0, 0.0);
            
        headerPtr = (HEAP_BASE + (imul(meshIndex, MESH_MEMORY_SIZE)|0))|0;
        modelPtr = HEAP32[(headerPtr + i_MODEL_STRUCT_PTR_OFFSET)>>2]|0;
        meshesPtr = HEAP32[(modelPtr + i_MODEL_MESHES_PTR_OFFSET)>>2]|0; 
        meshesLength = HEAP32[(modelPtr + i_MODEL_MESHES_LENGTH_OFFSET)>>2]|0;
        jointsPtr = HEAP32[(modelPtr + i_MODEL_JOINTS_PTR_OFFSET)>>2]|0;
        jointsLength = HEAP32[(modelPtr + i_MODEL_JOINTS_LENGTH_OFFSET)>>2]|0;
        vertexArrayPtr = HEAP32[(headerPtr + i_VERT_ARRAY_PTR_OFFSET)>>2]|0;
        
        //
        meshesLength = 1000;
        vertsLength = 1000;
        //
        for(i = 0; (i|0) < 1 /*(meshesLength|0)*/; i = (i + 1)|0) {
            meshPtr = (meshesPtr + (imul(i, MESH_STRUCT_SIZE)|0))|0;
            meshOffset = (HEAP32[(meshPtr + i_MESH_VERT_OFFSET_OFFSET)>>2]|0)<<2;
            meshOffset = (meshOffset + vertexArrayPtr)|0;
            vertsPtr = HEAP32[(meshPtr + i_MESH_VERTS_PTR_OFFSET)>>2]|0;
            vertsLength = HEAP32[(meshPtr + i_MESH_VERTS_LENGTH_OFFSET)>>2]|0;
            weightsPtr = HEAP32[(meshPtr + i_MESH_WEIGHTS_PTR_OFFSET)>>2]|0;
            weightsLength = HEAP32[(meshPtr + i_MESH_WEIGHTS_LENGTH_OFFSET)>>2]|0;
            
            // Calculate transformed vertices in the bind pose
            for(j = 0; (j|0) < 1 /*(vertsLength|0)*/; j = (j + 1)|0) {
                vertexPtr = ((imul(j, VERTEX_STRIDE)|0) + meshOffset)|0;
                vertPtr = (vertsPtr + (imul(j, VERT_STRUCT_SIZE)|0))|0;
                
                vx4 = SIMD_Float32x4_splat(toF(0));
                nx4 = SIMD_Float32x4_splat(toF(0));
                tx4 = SIMD_Float32x4_splat(toF(0));

                vertWeightsIndex = HEAP32[(vertPtr + i_VERT_WEIGHT_INDEX_OFFSET)>>2]|0;
                vertWeightsCount = HEAP32[(vertPtr + i_VERT_WEIGHT_COUNT_OFFSET)>>2]|0;
                for (k = 0; (k|0) < 1 /*(vertWeightsCount|0)*/; k = (k + 1)|0) {
                    weightPtr = (weightsPtr + imul((k + vertWeightsIndex|0)|0, WEIGHT_STRUCT_SIZE)|0)|0;
                    jointIndex = HEAP32[(weightPtr + i_WEIGHT_JOINT_INDEX_OFFSET)>>2]|0;
                    jointPtr = (jointsPtr + (imul(jointIndex, JOINT_STRUCT_SIZE)|0)|0);

                    // Rotate position
                    jointOrient = SIMD_Float32x4_load(HEAPU8, (jointPtr + f_JOINT_ORIENT_0_OFFSET)|0);
                    weightPos = SIMD_Float32x4_load(HEAPU8, (weightPtr + f_WEIGHT_POS_0_OFFSET)|0);
                    ix4 = SIMD_Float32x4_sub(
                        SIMD_Float32x4_add(
                            SIMD_Float32x4_mul(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 3, 3, 3, 0), tempx4),
                                               SIMD_Float32x4_swizzle(weightPos, 0, 1, 2, 0)),
                            SIMD_Float32x4_mul(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 1, 2, 0, 1), tempx4),
                                               SIMD_Float32x4_swizzle(weightPos, 2, 0, 1, 1))),
                        SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 2, 0, 1, 2),
                                           SIMD_Float32x4_swizzle(weightPos, 1, 2, 0, 2)));

                    rotatedPos = SIMD_Float32x4_add(
                        SIMD_Float32x4_sub(SIMD_Float32x4_mul(ix4, SIMD_Float32x4_swizzle(jointOrient, 3, 3, 3, 0)),
                                           SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 3, 3, 3, 0), jointOrient)),
                        SIMD_Float32x4_sub(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 2, 0, 1, 0), SIMD_Float32x4_swizzle(jointOrient, 1, 2, 0, 0)),
                                           SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 1, 2, 0, 0), SIMD_Float32x4_swizzle(jointOrient, 2, 0, 1, 0))));

                    jointPos = SIMD_Float32x4_load(HEAPU8, (jointPtr + f_JOINT_POS_0_OFFSET)|0);
                    weightBias = SIMD_Float32x4_swizzle(SIMD_Float32x4_load(HEAPU8, (weightPtr + f_WEIGHT_BIAS_OFFSET)|0), 0, 0, 0, 0);

                    // Translate position
                    vx4 = SIMD_Float32x4_add(vx4, SIMD_Float32x4_mul(SIMD_Float32x4_add(jointPos, rotatedPos), weightBias));

                    // Rotate Normal
                    weightNormal = SIMD_Float32x4_load(HEAPU8, (weightPtr + f_WEIGHT_NORMAL_0_OFFSET)|0);
                    ix4 = SIMD_Float32x4_sub(
                        SIMD_Float32x4_add(
                            SIMD_Float32x4_mul(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 3, 3, 3, 0), tempx4),
                                               SIMD_Float32x4_swizzle(weightNormal, 0, 1, 2, 0)),
                            SIMD_Float32x4_mul(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 1, 2, 0, 1), tempx4),
                                               SIMD_Float32x4_swizzle(weightNormal, 2, 0, 1, 1))),
                        SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 2, 0, 1, 2),
                                           SIMD_Float32x4_swizzle(weightNormal, 1, 2, 0, 2)));

                    rotatedPos = SIMD_Float32x4_add(
                        SIMD_Float32x4_sub(SIMD_Float32x4_mul(ix4, SIMD_Float32x4_swizzle(jointOrient, 3, 3, 3, 0)),
                                           SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 3, 3, 3, 0), jointOrient)),
                        SIMD_Float32x4_sub(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 2, 0, 1, 0), SIMD_Float32x4_swizzle(jointOrient, 1, 2, 0, 0)),
                                           SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 1, 2, 0, 0), SIMD_Float32x4_swizzle(jointOrient, 2, 0, 1, 0))));

                    nx4 = SIMD_Float32x4_add(nx4, SIMD_Float32x4_mul(rotatedPos, weightBias))

                    // Rotate Tangent
                    weightTangent = SIMD_Float32x4_load(HEAPU8, (weightPtr + f_WEIGHT_TANGENT_0_OFFSET)|0);
                    ix4 = SIMD_Float32x4_sub(
                        SIMD_Float32x4_add(
                            SIMD_Float32x4_mul(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 3, 3, 3, 0), tempx4),
                                               SIMD_Float32x4_swizzle(weightTangent, 0, 1, 2, 0)),
                            SIMD_Float32x4_mul(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 1, 2, 0, 1), tempx4),
                                               SIMD_Float32x4_swizzle(weightTangent, 2, 0, 1, 1))),
                        SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(jointOrient, 2, 0, 1, 2),
                                           SIMD_Float32x4_swizzle(weightTangent, 1, 2, 0, 2)));

                    rotatedPos = SIMD_Float32x4_add(
                        SIMD_Float32x4_sub(SIMD_Float32x4_mul(ix4, SIMD_Float32x4_swizzle(jointOrient, 3, 3, 3, 0)),
                                           SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 3, 3, 3, 0), jointOrient)),
                        SIMD_Float32x4_sub(SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 2, 0, 1, 0), SIMD_Float32x4_swizzle(jointOrient, 1, 2, 0, 0)),
                                           SIMD_Float32x4_mul(SIMD_Float32x4_swizzle(ix4, 1, 2, 0, 0), SIMD_Float32x4_swizzle(jointOrient, 2, 0, 1, 0))));

                    tx4 = SIMD_Float32x4_add(tx4, SIMD_Float32x4_mul(rotatedPos, weightBias))
                }

                // Position
                SIMD_Float32x4_store(HEAPU8, (vertexPtr + f_VERTEX_POS_0_OFFSET)|0, vx4);

                // TexCoord
                SIMD_Float32x4_store(HEAPU8, (vertexPtr + f_VERTEX_UV_0_OFFSET)|0, SIMD_Float32x4_load(HEAPU8, (vertPtr + f_VERT_TEXCOORD_0_OFFSET)|0));

                // Normal
                SIMD_Float32x4_store(HEAPU8, (vertexPtr + f_VERTEX_NORMAL_0_OFFSET)|0, nx4);

                // Tangent
                SIMD_Float32x4_store(HEAPU8, (vertexPtr + f_VERTEX_TANGENT_0_OFFSET)|0, tx4);
            }
        }
    }
    
    return {
        asmSkinSIMD: asmSkinSIMD,
    };
}

InitBuffer(buffer);

var asmSkinSIMD = _asmjsModuleSIMD(this, {}, buffer).asmSkinSIMD;
asmSkinSIMD(0);

PrintOutputBuffer(buffer);



