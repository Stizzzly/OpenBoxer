#pragma once
struct DamageSite {uint32_t call,returned,target;const char *name;unsigned id,count;unsigned pointers[4];bool scalar,body;};
inline constexpr DamageSite damageSites[]={
{0x1c70b,0x1c710,0x5ac10,"sub_45AC10",0,1,{16,0,0,0},false,false},
{0x1c740,0x1c745,0x5ac10,"sub_45AC10",0,1,{16,0,0,0},false,false},
{0x1c76f,0x1c774,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c77d,0x1c782,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c793,0x1c798,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c7a1,0x1c7a6,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c7bd,0x1c7c2,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c7cb,0x1c7d0,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c7e1,0x1c7e6,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c7ef,0x1c7f4,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c80a,0x1c80f,0x95b74,"_sqrt",2,2,{0,0,0,0},true,false},
{0x1c8be,0x1c8c3,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1c95e,0x1c963,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1cac6,0x1cacb,0x1abe,"sub_401ABE",3,2,{16,16,0,0},false,false},
{0x1cad6,0x1cadb,0x1f64,"sub_401F64",4,3,{16,16,0,0},false,false},
{0x1cb47,0x1cb4c,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1cb7b,0x1cb80,0x1f64,"sub_401F64",4,3,{16,16,0,0},false,false},
{0x1cbad,0x1cbb2,0x1839,"sub_401839",5,3,{0,0,0,0},false,false},
{0x1cbb8,0x1cbbd,0x1b1d,"sub_401B1D",6,0,{0,0,0,0},false,false},
{0x1cc19,0x1cc1e,0x9a750,"_rand",7,0,{0,0,0,0},false,false},
{0x1cc3e,0x1cc43,0x9a750,"_rand",7,0,{0,0,0,0},false,false},
{0x1cc67,0x1cc6c,0x9a750,"_rand",7,0,{0,0,0,0},false,false},
{0x1d775,0x1d77a,0x1839,"sub_401839",5,3,{0,0,0,0},false,false},
{0x1d7bf,0x1d7c4,0x5b780,"sub_45B780",8,1,{16,0,0,0},false,true},
{0x1d7e1,0x1d7e6,0x179e,"sub_40179E",9,0,{0,0,0,0},false,false},
{0x1d7ed,0x1d7f2,0x179e,"sub_40179E",9,0,{0,0,0,0},false,false},
{0x1d80b,0x1d810,0x179e,"sub_40179E",9,0,{0,0,0,0},false,false},
{0x1d817,0x1d81c,0x179e,"sub_40179E",9,0,{0,0,0,0},false,false},
{0x1d900,0x1d905,0x5ac10,"sub_45AC10",0,1,{16,0,0,0},false,false},
{0x1d92f,0x1d934,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1d949,0x1d94e,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1d95d,0x1d962,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1d977,0x1d97c,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1d98b,0x1d990,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1d9a5,0x1d9aa,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1d9e8,0x1d9ed,0x5b210,"sub_45B210",10,1,{16,0,0,0},false,false},
{0x1da04,0x1da09,0x9a750,"_rand",7,0,{0,0,0,0},false,false},
{0x1da3b,0x1da40,0x9a750,"_rand",7,0,{0,0,0,0},false,false},
{0x1da6c,0x1da71,0x9a750,"_rand",7,0,{0,0,0,0},false,false},
{0x1dc28,0x1dc2d,0x1839,"sub_401839",5,3,{0,0,0,0},false,false},
{0x1e037,0x1e03c,0x1d4d,"sub_401D4D",11,0,{0,0,0,0},false,false},
{0x1e04c,0x1e051,0x5ac10,"sub_45AC10",0,1,{16,0,0,0},false,false},
{0x1e079,0x1e07e,0x1839,"sub_401839",5,3,{0,0,0,0},false,false},
{0x1e08e,0x1e093,0x5b2e0,"sub_45B2E0",12,1,{16,0,0,0},false,true},
{0x1e0a3,0x1e0a8,0x5b210,"sub_45B210",10,1,{16,0,0,0},false,false},
{0x1e0e5,0x1e0ea,0x1014,"sub_401014",13,4,{0,0,0,0},true,false},
{0x1e0f0,0x1e0f5,0x11e0,"sub_4011E0",14,1,{0,0,0,0},true,false},
{0x1e109,0x1e10e,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1e145,0x1e14a,0x1cd0,"sub_401CD0",15,3,{16,16,16,0},false,false},
{0x1e17e,0x1e183,0x1f64,"sub_401F64",4,3,{16,16,0,0},false,false},
{0x1e1b2,0x1e1b7,0x5b690,"sub_45B690",16,1,{16,0,0,0},false,true},
{0x1e1bd,0x1e1c2,0x1b36,"sub_401B36",17,0,{0,0,0,0},true,false},
{0x1e1f4,0x1e1fa,0x0,"ds:__imp_alGetSourcei",18,3,{0,0,4,0},false,false},
{0x1e230,0x1e235,0x179e,"sub_40179E",9,0,{0,0,0,0},false,false},
{0x1e261,0x1e266,0x1eb0,"sub_401EB0",19,0,{0,0,0,0},false,false},
{0x1e26e,0x1e273,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1e27d,0x1e282,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1e294,0x1e299,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1e2a3,0x1e2a8,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1e2b8,0x1e2bd,0x95b74,"_sqrt",2,2,{0,0,0,0},true,false},
{0x1e2d5,0x1e2da,0x19c9,"sub_4019C9",1,1,{0,0,0,0},false,false},
{0x1e2f3,0x1e2f8,0x1f5f,"sub_401F5F",20,3,{16,0,16,0},false,false},
{0x1e305,0x1e30a,0x5b690,"sub_45B690",16,1,{16,0,0,0},false,true},
};
extern "C" {
void damage_observer0();
void damage_observer1();
void damage_observer2();
void damage_observer3();
void damage_observer4();
void damage_observer5();
void damage_observer6();
void damage_observer7();
void damage_observer8();
void damage_observer9();
void damage_observer10();
void damage_observer11();
void damage_observer12();
void damage_observer13();
void damage_observer14();
void damage_observer15();
void damage_observer16();
void damage_observer17();
void damage_observer18();
void damage_observer19();
void damage_observer20();
void damage_observer21();
void damage_observer22();
void damage_observer23();
void damage_observer24();
void damage_observer25();
void damage_observer26();
void damage_observer27();
void damage_observer28();
void damage_observer29();
void damage_observer30();
void damage_observer31();
void damage_observer32();
void damage_observer33();
void damage_observer34();
void damage_observer35();
void damage_observer36();
void damage_observer37();
void damage_observer38();
void damage_observer39();
void damage_observer40();
void damage_observer41();
void damage_observer42();
void damage_observer43();
void damage_observer44();
void damage_observer45();
void damage_observer46();
void damage_observer47();
void damage_observer48();
void damage_observer49();
void damage_observer50();
void damage_observer51();
void damage_observer52();
void damage_observer53();
void damage_observer54();
void damage_observer55();
void damage_observer56();
void damage_observer57();
void damage_observer58();
void damage_observer59();
void damage_observer60();
void damage_observer61();
void damage_observer62();
}
inline void *damageWrappers[]={reinterpret_cast<void*>(&damage_observer0),reinterpret_cast<void*>(&damage_observer1),reinterpret_cast<void*>(&damage_observer2),reinterpret_cast<void*>(&damage_observer3),reinterpret_cast<void*>(&damage_observer4),reinterpret_cast<void*>(&damage_observer5),reinterpret_cast<void*>(&damage_observer6),reinterpret_cast<void*>(&damage_observer7),reinterpret_cast<void*>(&damage_observer8),reinterpret_cast<void*>(&damage_observer9),reinterpret_cast<void*>(&damage_observer10),reinterpret_cast<void*>(&damage_observer11),reinterpret_cast<void*>(&damage_observer12),reinterpret_cast<void*>(&damage_observer13),reinterpret_cast<void*>(&damage_observer14),reinterpret_cast<void*>(&damage_observer15),reinterpret_cast<void*>(&damage_observer16),reinterpret_cast<void*>(&damage_observer17),reinterpret_cast<void*>(&damage_observer18),reinterpret_cast<void*>(&damage_observer19),reinterpret_cast<void*>(&damage_observer20),reinterpret_cast<void*>(&damage_observer21),reinterpret_cast<void*>(&damage_observer22),reinterpret_cast<void*>(&damage_observer23),reinterpret_cast<void*>(&damage_observer24),reinterpret_cast<void*>(&damage_observer25),reinterpret_cast<void*>(&damage_observer26),reinterpret_cast<void*>(&damage_observer27),reinterpret_cast<void*>(&damage_observer28),reinterpret_cast<void*>(&damage_observer29),reinterpret_cast<void*>(&damage_observer30),reinterpret_cast<void*>(&damage_observer31),reinterpret_cast<void*>(&damage_observer32),reinterpret_cast<void*>(&damage_observer33),reinterpret_cast<void*>(&damage_observer34),reinterpret_cast<void*>(&damage_observer35),reinterpret_cast<void*>(&damage_observer36),reinterpret_cast<void*>(&damage_observer37),reinterpret_cast<void*>(&damage_observer38),reinterpret_cast<void*>(&damage_observer39),reinterpret_cast<void*>(&damage_observer40),reinterpret_cast<void*>(&damage_observer41),reinterpret_cast<void*>(&damage_observer42),reinterpret_cast<void*>(&damage_observer43),reinterpret_cast<void*>(&damage_observer44),reinterpret_cast<void*>(&damage_observer45),reinterpret_cast<void*>(&damage_observer46),reinterpret_cast<void*>(&damage_observer47),reinterpret_cast<void*>(&damage_observer48),reinterpret_cast<void*>(&damage_observer49),reinterpret_cast<void*>(&damage_observer50),reinterpret_cast<void*>(&damage_observer51),reinterpret_cast<void*>(&damage_observer52),reinterpret_cast<void*>(&damage_observer53),reinterpret_cast<void*>(&damage_observer54),reinterpret_cast<void*>(&damage_observer55),reinterpret_cast<void*>(&damage_observer56),reinterpret_cast<void*>(&damage_observer57),reinterpret_cast<void*>(&damage_observer58),reinterpret_cast<void*>(&damage_observer59),reinterpret_cast<void*>(&damage_observer60),reinterpret_cast<void*>(&damage_observer61),reinterpret_cast<void*>(&damage_observer62)};
extern "C" {
void damage_script0();
void damage_script1();
void damage_script2();
void damage_script3();
void damage_script4();
void damage_script5();
void damage_script6();
void damage_script7();
void damage_script8();
void damage_script9();
void damage_script10();
void damage_script11();
void damage_script12();
void damage_script13();
void damage_script14();
void damage_script15();
void damage_script16();
void damage_script17();
void damage_script18();
void damage_script19();
void damage_script20();
void damage_script21();
void damage_script22();
void damage_script23();
void damage_script24();
void damage_script25();
void damage_script26();
void damage_script27();
void damage_script28();
void damage_script29();
void damage_script30();
void damage_script31();
void damage_script32();
void damage_script33();
void damage_script34();
void damage_script35();
void damage_script36();
void damage_script37();
void damage_script38();
void damage_script39();
void damage_script40();
void damage_script41();
void damage_script42();
void damage_script43();
void damage_script44();
void damage_script45();
void damage_script46();
void damage_script47();
void damage_script48();
void damage_script49();
void damage_script50();
void damage_script51();
void damage_script52();
void damage_script53();
void damage_script54();
void damage_script55();
void damage_script56();
void damage_script57();
void damage_script58();
void damage_script59();
void damage_script60();
void damage_script61();
void damage_script62();
}
inline void *damageScripts[]={reinterpret_cast<void*>(&damage_script0),reinterpret_cast<void*>(&damage_script1),reinterpret_cast<void*>(&damage_script2),reinterpret_cast<void*>(&damage_script3),reinterpret_cast<void*>(&damage_script4),reinterpret_cast<void*>(&damage_script5),reinterpret_cast<void*>(&damage_script6),reinterpret_cast<void*>(&damage_script7),reinterpret_cast<void*>(&damage_script8),reinterpret_cast<void*>(&damage_script9),reinterpret_cast<void*>(&damage_script10),reinterpret_cast<void*>(&damage_script11),reinterpret_cast<void*>(&damage_script12),reinterpret_cast<void*>(&damage_script13),reinterpret_cast<void*>(&damage_script14),reinterpret_cast<void*>(&damage_script15),reinterpret_cast<void*>(&damage_script16),reinterpret_cast<void*>(&damage_script17),reinterpret_cast<void*>(&damage_script18),reinterpret_cast<void*>(&damage_script19),reinterpret_cast<void*>(&damage_script20),reinterpret_cast<void*>(&damage_script21),reinterpret_cast<void*>(&damage_script22),reinterpret_cast<void*>(&damage_script23),reinterpret_cast<void*>(&damage_script24),reinterpret_cast<void*>(&damage_script25),reinterpret_cast<void*>(&damage_script26),reinterpret_cast<void*>(&damage_script27),reinterpret_cast<void*>(&damage_script28),reinterpret_cast<void*>(&damage_script29),reinterpret_cast<void*>(&damage_script30),reinterpret_cast<void*>(&damage_script31),reinterpret_cast<void*>(&damage_script32),reinterpret_cast<void*>(&damage_script33),reinterpret_cast<void*>(&damage_script34),reinterpret_cast<void*>(&damage_script35),reinterpret_cast<void*>(&damage_script36),reinterpret_cast<void*>(&damage_script37),reinterpret_cast<void*>(&damage_script38),reinterpret_cast<void*>(&damage_script39),reinterpret_cast<void*>(&damage_script40),reinterpret_cast<void*>(&damage_script41),reinterpret_cast<void*>(&damage_script42),reinterpret_cast<void*>(&damage_script43),reinterpret_cast<void*>(&damage_script44),reinterpret_cast<void*>(&damage_script45),reinterpret_cast<void*>(&damage_script46),reinterpret_cast<void*>(&damage_script47),reinterpret_cast<void*>(&damage_script48),reinterpret_cast<void*>(&damage_script49),reinterpret_cast<void*>(&damage_script50),reinterpret_cast<void*>(&damage_script51),reinterpret_cast<void*>(&damage_script52),reinterpret_cast<void*>(&damage_script53),reinterpret_cast<void*>(&damage_script54),reinterpret_cast<void*>(&damage_script55),reinterpret_cast<void*>(&damage_script56),reinterpret_cast<void*>(&damage_script57),reinterpret_cast<void*>(&damage_script58),reinterpret_cast<void*>(&damage_script59),reinterpret_cast<void*>(&damage_script60),reinterpret_cast<void*>(&damage_script61),reinterpret_cast<void*>(&damage_script62)};
