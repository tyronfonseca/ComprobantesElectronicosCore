// ARCHIVO GENERADO por gen_territorios.py -- NO EDITAR A MANO.
// Fuente: territorios.csv   (datos: 66572 bytes = nombres 50056 + tablas 16516)
#include <cstdint>
#include "cec/common/DivisionTerritorial.h"

#define DT_CON_BARRIOS 1

namespace cec::common {

namespace {

using NameOff = uint16_t;
constexpr NameOff kNone = 0xffff;  // sin nombre (hueco en la numeracion)

// Nombres UTF-8 separados por '\0' (deduplicados, con sufijos compartidos).
// Escapes octales de 3 digitos: no depende de la codificacion del archivo fuente.
constexpr char kNombres[] =
    "San Ram\303\263n (Cariblanca)\000Rinc\303\263n (Quebrada)\000San Luis (Florida)\000Gamalotillo 1 (Co"
    "lonia)\000Victoria (Alto Victoria)\000Ch\303\241nguena Centro (La Fila)\000Quebrada Seca (Santa Ana)"
    "\000Esperanza (Cantarrana)\000Filadelfia (Aguabuena)\000Santa Rosa (La Brisa)\000Vaca (Santa Rosa)"
    "\000Alto Cuen (Kjacka Bata)\000Concepci\303\263n (La Danta)\000Arenilla (Junta)\000Colinas Centro (M"
    "a\303\255z de los Uva)\000Mocho (Santa Luc\303\255a)\000Copey (Santa Luc\303\255a)\000Cantarrana (Sa"
    "nta Fe)\000Garc\303\255a Flamenco (Quebrada Grande)\000Akberie (Piedra Grande)\000Hu-Berie (Rancho G"
    "rande)\000Lotes (San Jorge)\000Animas (Dulce Nombre)\000Dos Bocas (Suerre)\000Cedral (Boquete)\000Mo"
    "i (San Vicente)\000Rabo de Mico (Aguacaliente)\000Sabanas (Barranco Parte)\000Lourdes (Rancho Ania) "
    "(parte)\000Pata de Gallo (San Crist\303\263bal) (parte)\000Sabanas (Barranco) (parte)\000Polvazales "
    "(Cerritos) (parte)\000Jalaca (parte)\000Hamaca (parte)\000Chirraca (parte)\000Cruz de Caravaca (part"
    "e)\000Cocaleca (parte)\000Eskameca (parte)\000Tierra Blanca (parte)\000Agua Blanca (parte)\000Aguabl"
    "anca (parte)\000Bajo Ca\303\261ada (parte)\000Quebrada Honda (parte)\000Paso Viga (parte)\000Taboga "
    "(parte)\000Mayorga (parte)\000Lucha (parte)\000Boca Providencia (parte)\000Valencia (parte)\000Santa"
    " Eulalia (parte)\000Santa Cecilia (parte)\000California (parte)\000Siberia (parte)\000Victoria (part"
    "e)\000Araucauria (parte)\000Casavieja (parte)\000Puebla (parte)\000La Fila (parte)\000Maravilla (par"
    "te)\000Alto Palma (parte)\000Poma (parte)\000Sabana (parte)\000Est\303\251fana (parte)\000Cantarrana"
    " (parte)\000Verbena (parte)\000Santa Elena (parte)\000Ballena (parte)\000Puente Arena (parte)\000Yer"
    "babuena (parte)\000Mandarina (parte)\000Quina (parte)\000Leona (parte)\000Canoa (parte)\000Santa Cla"
    "ra (parte)\000Puente Piedra (parte)\000Cabagra (parte)\000Real de Pereira (parte)\000Mar\303\255a Au"
    "xiliadora (parte)\000Aurora (parte)\000Altura (parte)\000Angostura (parte)\000Santa Rosa (parte)\000"
    "Dolorosa (parte)\000Monta\303\261a Rusa (parte)\000Isleta (parte)\000Quebrada Bonita (parte)\000Isla"
    " Bonita (parte)\000Villabonita (parte)\000Garita (parte)\000Uvita (parte)\000Ceiba Alta (parte)\000F"
    "rancisco Peralta (parte)\000Chonta (parte)\000Santa Marta (parte)\000Ojo de Agua (parte)\000Pato de "
    "Agua (parte)\000Selva (parte)\000Suiza (parte)\000Santa Luc\303\255a (parte)\000Echeverr\303\255a (p"
    "arte)\000Caba\303\261a (parte)\000Trinidad (parte)\000Quebrada Grande (parte)\000Acequia Grande (par"
    "te)\000Cuesta Grande (parte)\000Empalme (parte)\000Chompipe (parte)\000Rancho Alegre (parte)\000Mont"
    "ealegre (parte)\000Paso Machete (parte)\000Brillante (parte)\000Bajo Caliente (parte)\000Belo Horizo"
    "nte (parte)\000Santa Clara Norte (parte)\000Tuetal Norte (parte)\000Paso Tempisque (parte)\000Cariar"
    "i (parte)\000Tuba Creek (parte)\000Guayabal (parte)\000Surtubal (parte)\000Tremedal (parte)\000Naran"
    "jal (parte)\000Vega de Sardinal (parte)\000Mara\303\261onal (parte)\000Cedral (parte)\000Hospital (p"
    "arte)\000Zapotal (parte)\000Carrizal (parte)\000Brasil (parte)\000Montesol (parte)\000San Juan (part"
    "e)\000Carmen (parte)\000Rinc\303\263n Mes\303\251n (parte)\000Tur\303\255n (parte)\000Jarqu\303\255n"
    " (parte)\000Charc\303\263n (parte)\000Concepci\303\263n (parte)\000Divisi\303\263n (parte)\000Col"
    "\303\263n (parte)\000Vuelta Bols\303\263n (parte)\000Ca\303\261\303\263n (parte)\000Com\303\272n (pa"
    "rte)\000Guayabo (parte)\000Pac\303\255fico (parte)\000Chumico (parte)\000Caballo Blanco (parte)\000P"
    "rado (parte)\000Terr\303\263n Colorado (parte)\000San Gerardo (parte)\000Rodeo (parte)\000Cartago (p"
    "arte)\000Paso Macho (parte)\000Sitio (parte)\000Canoas Abajo (parte)\000Chirrip\303\263 Abajo (parte"
    ")\000Castillo (parte)\000Retallano (parte)\000Matasano (parte)\000San Cayetano (parte)\000Espino (pa"
    "rte)\000Montes de Oro (parte)\000San Isidro (parte)\000Bebedero (parte)\000Cerro Negro (parte)\000R"
    "\303\255o Negro (parte)\000Virgen del Socorro (parte)\000Huaso (parte)\000Para\303\255so (parte)\000"
    "San Josecito (parte)\000Coyolito (parte)\000Torito (parte)\000El Alto (parte)\000Salto (parte)\000Ga"
    "llo Pinto (parte)\000El Cristo (parte)\000Tejar (parte)\000Palmar (parte)\000Rohrmoser (parte)\000La"
    "s Huacas (parte)\000Barrancas (parte)\000Quebradas (parte)\000Bajo Venegas (parte)\000Vargas (parte)"
    "\000Delicias (parte)\000Medias (parte)\000Araucarias (parte)\000Lajas (parte)\000Lagunillas (parte)"
    "\000Aguas Buenas (parte)\000Cinco Esquinas (parte)\000Tres Piedras (parte)\000Aguas Gatas (parte)"
    "\000Juntas (parte)\000Aguas (parte)\000Bajo Ca\303\261as (parte)\000Salsipuedes (parte)\000Rosales ("
    "parte)\000\303\201ngeles (parte)\000Boca Gu\303\241piles (parte)\000Girasoles (parte)\000Mansiones ("
    "parte)\000Tablones (parte)\000Chompipes (parte)\000Ticalpes (parte)\000Purires (parte)\000Labores (p"
    "arte)\000Profesores (parte)\000San Marcos (parte)\000Ahogados (parte)\000Jabillos (parte)\000Monteci"
    "llos (parte)\000Pinos (parte)\000Bajo Matamoros (parte)\000Arbolitos (parte)\000Coto Brus (parte)"
    "\000Sol\303\255s (parte)\000Calle Quir\303\263s (parte)\000Monserrat (parte)\000Dent (parte)\000Cris"
    "to Rey (parte)\000Calle Hern\303\241ndez (parte)\000Jes\303\272s Jim\303\251nez (parte)\000Calle Ch"
    "\303\241vez (parte)\000P\303\241ez (parte)\000Quizarr\303\241 (parte)\000San Jos\303\251 (parte)\000"
    "Getseman\303\255 (parte)\000Isla Bonita(parte)\000Tigre (Caracol Norte)\000Bajo Pacuare (Norte)\000R"
    "\303\255o Nuevo (Norte)\000Ca\303\261ablancal (Este)\000Birr\303\255s (Este)\000Ca\303\261ablancal ("
    "Oeste)\000Miramar (Noroeste)\000Campo de Aterrizaje (Pueblo Triste)\000Alto Lari (Duri\303\261ak)"
    "\000El Campo (Guayabal)\000La Trinidad Centro (Guayabal)\000Tinamaste (San Cristobal)\000Sandial (Sa"
    "ndillal)\000Alto de Varas (Alto Varal)\000Alto Trinidad (Pu\303\261al)\000Bajos del Virilla (San Raf"
    "ael)\000Freeman (San Rafael)\000Tieso (San Rafael)\000Chirivico (Coopeisabel)\000Cerrillos (San Migu"
    "el)\000Durfuy (San Miguel)\000Gamalotillo 3 (Tierra F\303\251rtil)\000San Juan (Alto San Juan)\000Ca"
    "stilla (El Carmen)\000Fama (Carmen)\000San Lucas (Carmen)\000La Administraci\303\263n (Guti\303\251r"
    "rez Braun)\000San Josecito (Rinc\303\263n)\000Corea (Concepci\303\263n)\000Lourdes (Callej\303\263n)"
    "\000Dos de Tilar\303\241n (San Ram\303\263n)\000San Juan de Dios (Guabo)\000Los Pinos (Coliblanco)"
    "\000San Francisco (Tinoco)\000Vacablanca (San Francisco)\000Aquilea (San Francisco)\000San Rafael (R"
    "inc\303\263n Colorado)\000San Mart\303\255n (San Gerardo)\000Yolillal (San Antonio)\000San Juan (Que"
    "brada Palo)\000Tsipiri (Platanillo)\000Gamalotillo 2 (Gamalotillo)\000Varillas (Zapotillo)\000Porros"
    "at\303\255 (Paso Llano)\000Lindavista (Loma Gobierno)\000Tabla Grande (San Pedro)\000Coob\303\263 (P"
    "rogreso)\000San Juan de Mata Centro (Llano Bonito)\000Alto Jim\303\251nez (Montecristo)\000Picada (P"
    "ueblo Nuevo)\000Cabanga (Pueblo Nuevo)\000Buenavista (Katuir)\000San Mart\303\255n (Iraz\303\272 Sur"
    ")\000Bajo Pacuare (Sur)\000R\303\255o Nuevo (Sur)\000Santa Teresita Centro (Lajas)\000Pirr\303\255s "
    "(Las Parcelas)\000Pitalito Norte (Esquipulas)\000Para\303\255so (\303\201nimas)\000Sierra Centro (La"
    "s Minas)\000Santa Cecilia (Bajo Piedras)\000Bah\303\255a Drake Centro (Agujitas)\000L\303\255bano (F"
    "inanzas)\000Chepa (\303\201ngeles)\000Providencia (San Luis)\000Morelos (San Luis)\000Cerro Blanco ("
    "San Marcos)\000La Gloria (Los Mangos)\000Monte Fr\303\255o (Potrerillos)\000Alto Cerrillos (Coraz"
    "\303\263n de Jes\303\272s)\000Lim\303\263n 2000\000A\303\261o 2000\000Kil\303\263metro 10\000Kil\303"
    "\263metro 20\000Kil\303\263metro 40\000Colina 1\000Mleyuk 1\000Carb\303\263n 1\000Hatillo 1\000Kil"
    "\303\263metro 1\000Llano Bonito 1\000Tujankir 1\000Kil\303\263metro 51\000Coto 50-51\000Kil\303\263m"
    "etro 81\000Colina 2\000Mleyuk 2\000Carb\303\263n 2\000Hatillo 2\000Kil\303\263metro 2\000Llano Bonit"
    "o 2\000Tujankir 2\000Kil\303\263metro 22\000Coto 42\000Milla 52\000Milla 3\000Hatillo 3\000Kil\303"
    "\263metro 3\000N\303\255speros 3\000Milla 23\000Kil\303\263metro 33\000Coto 52-53\000Milla 4\000Hati"
    "llo 4\000Milla 14\000Milla 24\000Kil\303\263metro 24\000Coto 44\000Milla 5\000Hatillo 5\000Kil\303"
    "\263metro 5\000Milla 25\000Kil\303\263metro 25\000Coto 45\000Milla 6\000Hatillo 6\000Kil\303\263metr"
    "o 16\000Hatillo 7\000Kil\303\263metro 7\000Milla 27\000Kil\303\263metro 27\000Coto 47\000Hatillo 8"
    "\000Milla 28\000Milla 9\000Kil\303\263metro 9\000Kil\303\263metro 29\000Coto 49\000L\303\255nea B"
    "\000Juan Pablo II\000Le\303\263n XIII\000Alfonso XIII\000Juan XXIII\000INVU\000Guachalaba\000Vista d"
    "e T\303\251rraba\000Guaba\000La Ceiba\000Boca Ceiba\000Villas de la Ceiba\000Chirraca Arriba\000Lind"
    "a Arriba\000Guinea Arriba\000Tortuga Arriba\000Punta Mala Arriba\000Ca\303\261uela Arriba\000Jabilla"
    " Arriba\000Bonilla Arriba\000Potenciana Arriba\000Santa Elena Arriba\000Roble Arriba\000Pacuare Arri"
    "ba\000Pataste Arriba\000Camaronal Arriba\000San Rafael Arriba\000Santa Isabel Arriba\000\303\201ngel"
    " Arriba\000San Miguel Arriba\000San Juan Arriba\000Sam\303\251n Arriba\000Concepci\303\263n Arriba"
    "\000Ronr\303\263n Arriba\000Guayabo Arriba\000Ca\303\261o Seco Arriba\000Junquillo Arriba\000Canoas "
    "Arriba\000Mesetas Arriba\000Ca\303\261ales Arriba\000Chires Arriba\000Chirrip\303\263 Arriba\000Turr"
    "ialba\000Gamba\000Zamba\000Chamba\000Maramba\000Matamba\000Chimba\000La Bomba\000Caoba\000Bajo C\303"
    "\263rdoba\000Yerba\000R\303\255o Cuba\000Montes de Oca\000Saca\000Caracol de la Vaca\000Paso de la V"
    "aca\000Tarbaca\000Rojomaca\000Quebrada Seca\000Teca\000Guanacasteca\000Balalaica\000Jamaica\000Pac"
    "\303\255fica\000Isla Chica\000Bocachica\000Mojica\000Cer\303\241mica\000Dominica\000P\303\241nica"
    "\000Bot\303\241nica\000Santa M\303\263nica\000Pilarica\000\303\201frica\000Burica\000Am\303\251rica"
    "\000Mantica\000Fertica\000Polca\000Colonia Blanca\000Vara Blanca\000Tierra Blanca\000Casa Blanca\000"
    "Cuesta Blanca\000Playa Blanca\000Pe\303\261a Blanca\000Do\303\261a Blanca\000Hojablanca\000Copablanc"
    "a\000Casablanca\000Bajos de Ca\303\261ablanca\000Pe\303\261ablanca\000Talamanca\000Villafranca\000Bo"
    "ca de Barranca\000San Antonio de Barranca\000Bajo Barranca\000Penca\000La Roca\000Ococa\000Gandoca"
    "\000Cocoroca\000Rinc\303\263n de Abarca\000Bajo Abarca\000Comarca\000Chirca\000Palma de Mallorca\000"
    "Bajos de Urasca\000Alto Fresca\000Mosca\000Yuca\000Machuca\000Sonafluca\000R\303\255o Uruca\000Ma"
    "\303\255z de Boruca\000Churuca\000Picada\000Moncada\000Cascada\000Quijada\000Dibujada\000Playa Pelad"
    "a\000Inmaculada\000Calle Amada\000Palma Quemada\000Granada\000Cuesta Venada\000Rinconada\000Hondonad"
    "a\000Peoresnada\000Boca Tapada\000Quebrada\000Palma Dorada\000Alborada\000Tierra Colorada\000Cuesta "
    "Colorada\000Estrada\000Playa Quesada\000Bajo Quesada\000Baltazar Quesada\000Piedra Pintada\000Pochot"
    "ada\000Aguada\000Pita Rayada\000Cruzada\000Ca\303\261ada\000Cereceda\000Artieda\000Arboleda\000Alame"
    "da\000Andr\303\263meda\000Vereda\000Rueda\000B\303\263veda\000Florida\000Tierra Prometida\000La Esme"
    "ralda\000Alto Esmeralda\000Giralda\000Hilda\000Holanda\000Yolanda\000Irlanda\000Senda\000Loma Linda"
    "\000Vista Linda\000Hacienda Playa Linda\000Villalinda\000Lomalinda\000Bajo Quebrada Honda\000Ronda"
    "\000Volconda\000Sabana Redonda\000Mata Redonda\000Poza Redonda\000Pilahonda\000Rotonda\000Bajo La So"
    "rda\000La Aldea\000Orqu\303\255dea\000Goicoechea\000Cochea\000Galilea\000Atenea\000Fila Guinea\000Ha"
    "cienda Mata de Guinea\000Corea\000Jicotea\000Bajos de Dorotea\000Garrafa\000Alfa\000Bajos de Praga"
    "\000Rinconada Vega\000Bajo Vega\000Condega\000I Griega\000Gallega\000Omega\000Chorotega\000Ortega"
    "\000Coopevega\000Ortiga\000Bajo Z\303\272\303\261iga\000Ganga\000Cabanga\000Pachanga\000Bajo Malanga"
    "\000Cananga\000Talolinga\000Bonga\000Taboga\000Coopetaboga\000Loma Larga\000Sabanalarga\000Mayorga"
    "\000Isla Tortuga\000Cayuga\000Macha\000Guaracha\000Kichuguecha\000Turubokicha\000Sor\303\263kicha"
    "\000Hojancha\000Concha\000Hacienda La Moncha\000Roncha\000Ceiba Mocha\000Trocha\000Vivienda en March"
    "a\000La Lucha\000Nueva Lucha\000Alpha\000Arancibia\000Altagracia\000Moracia\000Alsacia\000Venecia"
    "\000Grecia\000Galicia\000Abundancia\000Numancia\000Francia\000Estancia\000Larga Distancia\000Constan"
    "cia\000Providencia\000Independencia\000Valencia\000Santa Clemencia\000Florencia\000Escocia\000Tabarc"
    "ia\000Murcia\000Heredia\000La Lidia\000La India\000Tom\303\241s Guardia\000Concordia\000Filadelfia"
    "\000Villa Ligia\000Liburgia\000Sukia\000Dalia\000Westfalia\000Santa Eulalia\000Floralia\000Tesalia"
    "\000Celia\000La Delia\000Santa Rogelia\000Amelia\000Cornelia\000Santa Cecilia\000Calle Emilia\000Sag"
    "rada Familia\000Sacra Familia\000Brasilia\000Magnolia\000Julia\000Ania\000Albania\000Solania\000Alto"
    " Alemania\000Germania\000Birmania\000Hispania\000Betania\000Gardenia\000Armenia\000Higinia\000Nueva "
    "Virginia\000Flaminia\000Sonia\000Valle Vasconia\000Dondonia\000Begonia\000Babilonia\000La Colonia"
    "\000Polonia\000Daytonia\000California\000Candelaria\000Gran Samaria\000La Guaria\000Coopeguaria\000M"
    "onse\303\261or Sanabria\000Port\303\263n Iberia\000Liberia\000Agua Agria\000Cabiria\000Biarquiria"
    "\000La Gloria\000Bajo Gloria\000Alto Gloria\000La Victoria\000Colonia Victoria\000R\303\255o Victori"
    "a\000Coopevictoria\000Chimurria\000Curia\000Manchuria\000Santa Rita de Casia\000Hortensia\000Helveti"
    "a\000Jardines de Moravia\000Artavia\000Olivia\000Bolivia\000Segovia\000Kezia\000Ceiba Baja\000La Caj"
    "a\000Casa de Teja\000Boca Vieja\000Hacienda Vieja\000Zagala Vieja\000M\303\241quina Vieja\000Garita "
    "Vieja\000Pitahaya Vieja\000Calle Vieja\000Cangreja\000Sitio Botija\000La Granja\000Cuesta Roja\000Cr"
    "uz Roja\000Rioja\000Purruja\000Brunka\000Doka\000Alaska\000Suretka\000Punta Mala\000Bengala\000Guate"
    "mala\000Upala\000Zavala\000Ayala\000Tabla\000Puebla\000Rambla\000Santa Tecla\000Cascabela\000Bernabe"
    "la\000Loma Caucela\000Piedra de Candela\000Varela\000Abuela\000Finca Alajuela\000Plazuela\000Ca\303"
    "\261uela\000Pi\303\261uela\000Fila\000La Leila\000Manila\000Aguila\000Barro de Olla\000Marbella\000V"
    "alle La Estrella\000Boca R\303\255o Estrella\000Marsella\000Botella\000Jabilla\000Barbilla\000Florec"
    "illa\000Vicente Badilla\000Bajo Badilla\000Alto Cebadilla\000Granadilla\000Espadilla\000Alto Quebrad"
    "illa\000Ardilla\000Cuesta Chinchilla\000Cruce Chinchilla\000Rinc\303\263n de Sabanilla\000Guadalupe "
    "o Arenilla\000San Jos\303\251 Pinilla\000Vainilla\000Matinilla\000Bajos de Bonilla\000Esquivel Bonil"
    "la\000Lagunilla\000Capilla\000Quebrada Amarilla\000Piedra Amarilla\000Casa Amarilla\000Banderilla"
    "\000Vuelta del Virilla\000Masilla\000Losilla\000Setilla\000Ca\303\261a Castilla\000Puerto Castilla"
    "\000Barranquilla\000Maravilla\000Sevilla\000Bola\000Lola\000Boca Sixaola\000Chibola\000Carambola\000"
    "Cocacola\000Pangola\000Do\303\261a Fabiola\000Cabriola\000Banamola\000Amapola\000Caspirola\000Artola"
    "\000Loyola\000Rampla\000Alto Perla\000Casorla\000Bajo Isla\000Ca\303\261af\303\255stula\000Yama\000J"
    "uanilama\000Guadarrama\000Matama\000Alto Cima\000Bajo Lima\000Gu\303\241cima\000Ulima\000Colima\000F"
    "\303\241tima\000Mar\303\255tima\000Playa Palma\000Bajo Palma\000Alto Palma\000La Loma\000Poma\000Vil"
    "la Roma\000Jardines de Roma\000Alto Pie de Paloma\000Geroma\000Coroma\000Irma\000Numa\000Chuma\000Mo"
    "ntezuma\000Santa Ana\000Costa Ana\000Roble de Sabana\000Tropicana\000Americana\000Bocana\000Palangan"
    "a\000Colombiana\000Potenciana\000Luciana\000Alto Herediana\000Damiana\000Mariana\000Rancho Floriana"
    "\000Luisiana\000Vista Llana\000Avellana\000Vuelta Campana\000Cantarrana\000Cerbatana\000Juan Santana"
    "\000Villa Quintana\000Fontana\000Santa Juana\000Iguana\000Roxana\000Alto Mena\000Tena\000Verbena\000"
    "Azucena\000Cartagena\000Santa Elena\000Do\303\261a Elena\000Bah\303\255a Ballena\000Arena\000Macaren"
    "a\000Sirena\000Lorena\000Carrera Buena\000Yerbabuena\000Copabuena\000Tierrabuena\000Campo de Aguabue"
    "na\000Nochebuena\000Vegas de Ch\303\241nguena\000La Mina\000Playa Mina\000Do\303\261a Nina\000Tirimb"
    "ina\000Turbina\000Encina\000Sardina\000Mar\303\255a Reina\000Josefina\000Jabalina\000Santa Catalina"
    "\000Celina\000Angelina\000Gallina\000Pita Villa Colina\000Molina\000Carolina\000Paulina\000Puente Gu"
    "illermina\000Boca Parismina\000Marina\000Catarina\000Peregrina\000Corina\000Uni\303\263n Campesina"
    "\000Boca R\303\255o Matina\000Betina\000Villa Argentina\000Orotina\000Palestina\000Quina\000Joaquina"
    "\000M\303\241quina\000Punta Mona\000La Zona\000Zumbona\000Macacona\000Punta Leona\000Bajo Barahona"
    "\000Nueva Cinchona\000Tejona\000Gongolona\000Islona\000Carmona\000Llorona\000La Casona\000Pavona\000"
    "Arizona\000Rancho Luna\000Puna\000Bajo Laguna\000La Fortuna\000Balboa\000Rinc\303\263n Gamboa\000Baj"
    "o Gamboa\000Carlos Mar\303\255a Ulloa\000Canoa\000Figueroa\000Cuipilapa\000Tacasolapa\000Bajo Grapa"
    "\000Popoyuapa\000Acoyapa\000Arepa\000Chiripa\000Milpa\000Pampa\000La Trampa\000Zompopa\000Europa\000"
    "Chispa\000Cabeceras de R\303\255o Ora\000Cerro Nara\000Grano de Oro o Xara\000Sabara\000Bajo Santa B"
    "\303\241rbara\000Sahara\000Guadalajara\000La Clara\000Santa Clara\000S\303\241mara\000Carara\000Vist"
    "a Nosara\000Playa Nosara\000Bocas de Nosara\000Bajo Guevara\000Villas de Darizara\000Bajo Cabra\000P"
    "uerto Culebra\000Alhambra\000Alfombra\000Escuadra\000La Piedra\000Guardianes de la Piedra\000Cuesta "
    "de Piedra\000Puente de Piedra\000Corral de Piedra\000Llano Piedra\000Alondra\000Punta de Cera\000La "
    "Ribera\000Choricera\000Pradera\000Caldera\000Bandera\000Cachera\000Ranchera\000Chanchera\000Trincher"
    "a\000Riviera\000Calera\000Galera\000Tabacalera\000Alto Escalera\000Nopalera\000Tablera\000Gallera"
    "\000Bajo Ladrillera\000Coyolera\000Hulera\000Palmera\000Ranera\000Platanera\000Cartonera\000Bajo Sap"
    "era\000Cabrera\000Morera\000Vuelta Herrera\000Rinc\303\263n de Herrera\000Calle Herrera\000Bajo Herr"
    "era\000Chorrera\000Quesera\000Fomentera\000Carpintera\000Gotera\000Coyotera\000Frutera\000Paquera"
    "\000Tranquera\000Bloquera\000Clavera\000Calavera\000Primavera\000La Rivera\000Pi\303\261era\000San R"
    "afael de Cabagra\000Fila Negra\000Playa Negra\000Tigra\000Lira\000Punto de Mira\000Guaira\000Lotes M"
    "oreira\000Sequeira\000Chira\000Altamira\000Palmira\000Capira\000Katira\000Dora\000Jora\000La Mora"
    "\000Cuesta Mora\000Mata de Mora\000Rinc\303\263n de Mora\000Altos de Mora\000El Mora\000Alto Macho M"
    "ora\000Bajo Mora\000Calle Los Mora\000Alto Los Mora\000V\303\255bora\000Tacora\000Isla Cocora\000Poc"
    "ora\000Mar\303\255a Auxiliadora\000Tronadora\000Isidora\000Pandora\000Argendora\000Lindora\000Bajo Z"
    "amora\000Jes\303\272s Jim\303\251nez Zamora\000Coopezamora\000Sonora\000Valle La Aurora\000Alto Past"
    "ora\000Navarra\000La Sierra\000Quintanar de la Sierra\000Alto Sierra\000Para\303\255so de la Tierra"
    "\000Puente de Tierra\000Guerra\000Isla del Cura\000Ca\303\261a Dura\000Playa Herradura\000Segura\000"
    "Pel\303\263n de la Bajura\000Vistas de la Llanura\000Loma de la Altura\000San Buenaventura\000Alto M"
    "ontura\000Angostura\000Osa\000La Casa\000Puente Casa\000Mesa\000Oracabesa\000Marsellesa\000Presa\000"
    "Santa Teresa\000Represa\000Isla Sorpresa\000Brisa\000La Luisa\000Santa Luisa\000Mar\303\255a Luisa"
    "\000Balsa\000Bolsa\000Santa Rosa\000Bosques de Do\303\261a Rosa\000Lajosa\000Villa Hermosa\000Vista "
    "Hermosa\000Playa Hermosa\000Villahermosa\000Formosa\000Mariposa\000Ponderosa\000Barrosa\000Toyosa"
    "\000Saprissa\000San Juan de Mata\000Sitio Mata\000Gu\303\241bata\000Monte Plata\000Casamata\000Zapat"
    "a\000Catarata\000La Julieta\000Bajo Arrieta\000La Cajeta\000Violeta\000Plazoleta\000Barroeta\000La C"
    "arreta\000Meseta\000La Pita\000Bajo Pita\000Santa Rita\000Virgencita\000Isla Jesucita\000Haciendita"
    "\000Paso Beita\000La Luchita\000Cajita\000Zanjita\000Vistas de Alajuelita\000Villita\000Espa\303\261"
    "olita\000Perlita\000Islita\000Damita\000Palmita\000Anita\000Bananita\000Iguanita\000Venita\000Changu"
    "enita\000Marinita\000Quebrada Bonita\000Villa Bonita\000Isla Bonita\000Loma Bonita\000Sabana Bonita"
    "\000Bajo Bonita\000Colonia Menonita\000Fortunita\000Lupita\000La Garita\000Calle Garita\000Estero Ga"
    "rita\000Chacarita\000La Margarita\000Santa Margarita\000Clarita\000Candelarita\000Camarita\000Bel"
    "\303\251n de Nosarita\000Llanos de Altamirita\000Morita\000Florita\000Boca del Parrita\000Purita\000"
    "Santa Teresita\000Cieneguita\000Talolinguita\000Taboguita\000Cahuita\000Paquita\000Alto Vaquita\000P"
    "laya Chiquita\000Uvita\000Ca\303\261ita\000Monta\303\261ita\000Pe\303\261ita\000Bajo Leg\303\274ita"
    "\000Piedra Alta\000Torre Alta\000Villalta\000Altos de Peralta\000Riberalta\000Delta\000La Vuelta\000"
    "Media Vuelta\000Gavil\303\241n Canta\000Danta\000Giganta\000Planta\000Atalanta\000Taranta\000Torment"
    "a\000Paso La Tinta\000Quinta\000Chonta\000Junta\000Repunta\000Dota\000Mota\000Pochota\000Coyota\000S"
    "anta Marta\000Berta\000Agua Muerta\000Vesta\000Floresta\000La Cuesta\000Linda Vista\000Jardines de A"
    "utopista\000Villa Solidarista\000La Conquista\000Lindavista\000Bellavista\000Playa Buenavista\000Cer"
    "ro Buenavista\000Alto Buenavista\000Acosta\000Pilangosta\000Muta\000Piuta\000Chiruta\000Altos de Pas"
    "cua\000Padua\000Finca Agua\000Patio de Agua\000Ojo de Agua\000Pozo de Agua\000Chachagua\000Cuesta Bi"
    "jagua\000Managua\000Caragua\000Doboncragua\000Brisas de Veragua\000Piragua\000Cacoragua\000Bajo Legu"
    "a\000Antigua\000Coopevigua\000Kerkua\000Eva\000Punta Uva\000Bocabrava\000Coopeoctava\000Japdeva\000C"
    "ueva\000Zagala Nueva\000Villa Nueva\000Isla Nueva\000Villanueva\000Chiva\000La Cooperativa\000Selva"
    "\000Madreselva\000Villanova\000Terranova\000Legua de Barva\000Minerva\000Reserva\000Curva\000Maya"
    "\000Raya\000La Pacaya\000Vizcaya\000Cuesta Pitahaya\000Puesto La Playa\000Guacamaya\000Alto Araya"
    "\000Vargas Araya\000Masaya\000Sictaya\000Quisqueya\000Joya\000Moya\000Claraboya\000Camboya\000Nicoya"
    "\000Troya\000Otoya\000Playa Cabuya\000Peraza\000Ca\303\261aza\000Alto Corteza\000Niza\000Bajo Loaiza"
    "\000Ceniza\000La Suiza\000Punta de Lanza\000Confianza\000Bonanza\000Colonia Esperanza\000Nuestra Se"
    "\303\261ora de la Esperanza\000Porvenir de la Esperanza\000Villa Esperanza\000Santa Esperanza\000Nue"
    "va Esperanza\000Oasis de Esperanza\000Carranza\000Rinc\303\263n de Zaragoza\000Bajo Espinoza\000Cabe"
    "ceras de Garza\000\303\201ngeles de Garza\000Esparza\000Lechuza\000Guatuza\000La L\303\255a\000Alto "
    "Blanca Luc\303\255a\000Llanos de Santa Luc\303\255a\000Cofrad\303\255a\000Alto Vig\303\255a\000Bah"
    "\303\255a\000Tal\303\255a\000Rosal\303\255a\000Agon\303\255a\000Villa Mar\303\255a\000Santa Mar\303"
    "\255a\000Coraz\303\263n de Mar\303\255a\000Jes\303\272s Mar\303\255a\000Juan Santamar\303\255a\000Ch"
    "itar\303\255a\000Quitar\303\255a\000Alegr\303\255a\000Dir\303\255a\000Lor\303\255a\000Lis\303\255mac"
    "o Chavarr\303\255a\000Maiquet\303\255a\000Calle Tranv\303\255a\000Mata de Ca\303\261a\000Caba\303"
    "\261a\000Espa\303\261a\000Cara\303\261a\000San Jos\303\251 de la Monta\303\261a\000R\303\255o Monta"
    "\303\261a\000Pe\303\261a\000Colonia Naranje\303\261a\000Colonia Puriscale\303\261a\000Bernardo Ure"
    "\303\261a\000Turrucare\303\261a\000Colonia Palmare\303\261a\000Colonia Isidre\303\261a\000Cure\303"
    "\261a\000Campi\303\261a\000Crori\303\261a\000Catalu\303\261a\000Gar\303\272a\000Alto P\303\251rez As"
    "t\303\272a\000Shewab\000Nagatac\000Lomas de Tepeyac\000Chapultepec\000Tayutic\000Soledad\000Jardines"
    " La Trinidad\000Colonia Trinidad\000Sant\303\255sima Trinidad\000Altos de Trinidad\000Comunidad\000S"
    "alubridad\000Navidad\000Colonia Libertad\000La Amistad\000Laguna; Merced\000Freehold\000Maryland\000"
    "Hollywood\000Maynard\000Strafford\000Santa Fe\000Sitrae\000Tonjibe\000Zeta Trece\000Belice\000Finca "
    "Zona Once\000Avance\000Lince\000Bajo Los Arce\000Sauce\000El Cruce\000Jade\000Ugalde\000Quebrada Gra"
    "nde\000Isla Grande\000Sabana Grande\000Piedra Grande\000Mata Grande\000Anita Grande\000Cuesta Grande"
    "\000Playa Grande\000Monta\303\261a Grande\000San Juan Grande\000Rinc\303\263n Grande\000Rancho Grand"
    "e\000Llano Grande\000Potrero Grande\000Bajos del R\303\255o Grande\000Palogrande\000Isla Verde\000Lo"
    "ma Verde\000Vista Verde\000Selva Verde\000Valle Verde\000Monte Verde\000Balc\303\263n Verde\000Cangr"
    "ejo Verde\000Palo Verde\000Llano Verde\000Campo Verde\000Cerro Verde\000Junta de R\303\255o Verde"
    "\000Villaverde\000Monteverde\000Bajo Valverde\000Rinc\303\263n de Monge\000Bajo Monge\000Pedrito Mon"
    "ge\000Luis Monge\000Los Monge\000San Gerardo (parte) San Jorge\000Cambalache\000Bajo Trapiche\000Uri"
    "che\000Malinche\000Mala Noche\000Ojoche\000Cuesta Malanoche\000Coyoche\000Rothe\000Ast\303\272a-Piri"
    "e\000Puerto Peje\000R\303\255o Peje\000Pijije\000Bah\303\255a Drake\000Calle Roble\000El Roble\000Va"
    "ra del Roble\000Flor del Roble\000Chile\000La Salle\000El Valle\000Vista del Valle\000Vistas del Val"
    "le\000Rangalle\000Muelle\000Lesville\000Ra\303\255z de Hule\000El Hule\000Don Jaime\000Curime\000Emp"
    "alme\000Nacaome\000Alto June\000San Felipe\000Sacatipe\000Portegolpe\000Sierpe\000Dururpe\000Guadalu"
    "pe\000Castelmare\000Boca Pacuare\000Bocuare\000Nanabre\000Cuabre\000Gibre\000Ocho de Diciembre\000Ve"
    "intiocho de Diciembre\000Quince de Setiembre\000Veinte de Noviembre\000Dulce Nombre\000Alto Alumbre"
    "\000Sucre\000R\303\255o Madre\000Comadre\000Vegas de Cerere\000Chircagre\000Piedra Alegre\000Vista A"
    "legre\000Campo Alegre\000Puerto Alegre\000Trejos Montealegre\000Savegre\000Fila Tigre\000Mano de Tig"
    "re\000Bajos del Tigre\000Bajo Tigre\000Alto Tigre\000Telire\000Tarire\000Baltimore\000Suerre\000Chiv"
    "erre\000Curirre\000La Torre\000Puente de Salitre\000Balastre\000Nandayure\000La Ese\000Chase\000Tari"
    "se\000Combate\000Fila Aguacate\000Cuesta Aguacate\000Altos del Aguacate\000Isla Berrugate\000Rinc"
    "\303\263n de Ulate\000Bajos de Chilamate\000Tempate\000El Mastate\000Finca Zona Siete\000Ramal Siete"
    "\000Canalete\000Martillete\000Morete\000Portete\000Boquete\000Nancite\000Chag\303\274ite\000Gigante"
    "\000Escalante\000Diamante\000Bajo Bustamante\000Cuadrante\000San Vicente\000Punta Caliente\000Rinc"
    "\303\263n Caliente\000Bajo Caliente\000Aguacaliente\000Bello Oriente\000San Clemente\000Anselmo Llor"
    "ente\000La Fuente\000Lapuente\000Altos de Conte\000Boca del Monte\000Vista del Monte\000Santiago del"
    " Monte\000San Antonio del Monte\000Piamonte\000Bellomonte\000Desmonte\000Bello Horizonte\000Paso Bot"
    "e\000Cote\000Bajos Jicote\000Cuesta Jocote\000Mogote\000Pochote\000Piedra Achiote\000Bajo Achiote"
    "\000Alto Achiote\000Agualote\000Bajo Zopilote\000Quebrada Zapote\000Fila Zapote\000San Isidro de Zap"
    "ote\000Guapote\000Sonzapote\000Morote\000Garrote\000Loma Ayote\000Playa Coyote\000San Francisco de C"
    "oyote\000Bajos del Coyote\000Puerto Coyote\000San Jos\303\251 o Pizote\000Muerte\000Suerte\000Villaf"
    "uerte\000Corte\000Arancibia Norte\000Filadelfia Norte\000Granadilla Norte\000Verbena Norte\000Barra "
    "de Matina Norte\000Chacarita Norte\000Favorita Norte\000Patio de Agua Norte\000Esperanza Norte\000Mo"
    "rote Norte\000San Crist\303\263bal Norte\000Cedral Norte\000San Rafael Norte\000Linda del Norte\000F"
    "lor del Norte\000Lomas del Norte\000San Juan Norte\000San Mart\303\255n Norte\000Misi\303\263n Norte"
    "\000San Ram\303\263n Norte\000Bananito Norte\000Encanto Norte\000Palmar Norte\000Cuatro Esquinas Nor"
    "te\000Piedades Norte\000Mercedes Norte\000\303\201ngeles Norte\000Santa Gertrudis Norte\000Cartagos "
    "Norte\000Colegios Norte\000San Jos\303\251 Norte\000Sarch\303\255 Norte\000Ceiba Este\000Luisa Este"
    "\000Colinas del Este\000Prados del Este\000San Miguel Este\000R\303\255o Blanco Este\000Barra del Co"
    "lorado Este\000Cuatro Esquinas Este\000Campo Tres Este\000Esterillos Este\000Finca Guanacaste\000Cru"
    "ce Guanacaste\000Paso Guanacaste\000Tinamaste\000Pataste\000Pandora Oeste\000Luisa Oeste\000Corozal "
    "Oeste\000Residencial del Oeste\000San Miguel Oeste\000R\303\255o Blanco Oeste\000Campo Tres Oeste"
    "\000Esterillos Oeste\000R\303\255o Celeste\000Piedades Noroeste\000Faro del Suroeste\000Miramar Sure"
    "ste\000Leyte\000Sepeque\000Cacique\000Coocique\000Tucurrique\000Tanque\000Palenque\000San Roque\000P"
    "arque\000Bosque\000Gonz\303\241lez Truque\000Piave\000Milla Nueve\000Golden Grove\000Pejibaye\000Mac"
    "ho Gaff\000Fourth Cliff\000Barnstorf\000Gregg\000Sterling\000Irving\000Williamsburg\000Orlich\000Cue"
    "sta Winch\000Ten Switch\000Vereh\000Colonia Nazareth\000Longo Mai\000Kekoldi\000Carrandi\000Bajo Ble"
    "i\000Cachabli\000Goli\000Metr\303\263poli\000Musmanni\000Colombari\000Cariari\000Coopecariari\000Amu"
    "bri\000Yheri\000Anateri\000Calveri\000Singri\000Suiri\000Camaquiri\000Nadori\000Capri\000Tobosi\000O"
    "rosi\000Villa Rossi\000Katsi\000Uatsi\000Bratsi\000Yurusti\000Asovigui\000Jekui\000Bagatzi\000Waldec"
    "k\000Mile Creek\000Hone Creek\000Hotel Creek\000Ana Frank\000Luna Park\000Nueva York\000Tobruk\000Cu"
    "esta Fila de Cal\000Guayabal\000Escobal\000San Cristobal\000Santubal\000Alto Surtubal\000San Crist"
    "\303\263bal\000Icacal\000Tacacal\000Chirracal\000Loma Guacal\000Huacal\000Dominical\000Tropical\000C"
    "a\303\261ablancal\000Pencal\000Maquencal\000Cocal\000Bodocal\000Percal\000Pe\303\261ascal\000Purisca"
    "l\000Campabadal\000Robledal\000Barbudal\000Loma Real\000Vista Real\000Playa Real\000Paso Real\000Gui"
    "neal\000Barreal\000Villarreal\000Monterreal\000Bureal\000Pedregal\000Madrigal\000Congal\000Nogal\000"
    "Sabogal\000Tortugal\000Bah\303\255a Chal\000Palmichal\000Marichal\000Conchal\000Ojochal\000Colonial"
    "\000R\303\255o Guarial\000Barrial\000Burial\000Manantial\000Parroquial\000Cascajal\000Cangrejal\000N"
    "aranjal\000Ajenjal\000Turrujal\000Mallal\000Millal\000Maicillal\000Jaboncillal\000Ratoncillal\000Rod"
    "illal\000Yolillal\000Platanillal\000Tornillal\000Trompillal\000Varillal\000Cerrillal\000Setillal\000"
    "Zapotillal\000Junquillal\000Guacimal\000Bajo de Guarumal\000Alto Guarumal\000Boca Arenal\000Boca Sar"
    "dinal\000Sahinal\000Jazminal\000Espinal\000Jabonal\000Carbonal\000Tronconal\000Ostional\000Abejonal"
    "\000San Isidro de Limonal\000Camaronal\000Higueronal\000Pedernal\000Chapernal\000Copal\000Jicaral"
    "\000Guagaral\000Alto Sajaral\000Cedral\000Catedral\000Maderal\000Romeral\000El General\000San Isidro"
    " de el General\000Jilgueral\000Ca\303\261averal\000Caragral\000Alto Chiral\000Moral\000Titoral\000Pa"
    "storal\000Chaparral\000Chiverral\000Corral\000Purral\000Salitral\000La Central\000Jarasal\000Parasal"
    "\000Cipresal\000Balsal\000Rosal\000Guastomatal\000Tempatal\000Mastatal\000Cafetal\000Pital\000Nancit"
    "al\000Caimital\000Palmital\000Capital\000Hospital\000Cerital\000Coquital\000Occidental\000Guaria Ori"
    "ental\000Ocotal\000Tacotal\000Jocotal\000Pochotal\000Cuesta Achiotal\000Gamalotal\000Bajos de Zapota"
    "l\000Bajo Zapotal\000Alto Zapotal\000R\303\255o Zapotal\000Mozotal\000Alto Portal\000Murtal\000Crist"
    "al\000Cambutal\000Tagual\000Bajo Bijagual\000Villaval\000Claraval\000Sandoval\000Pacayal\000Copeyal"
    "\000Cabuyal\000Quizarrazal\000Polvazal\000Cortezal\000Bajo Cornizal\000Alto Carrizal\000Corozal\000P"
    "orozal\000Quetzal\000Bambuzal\000Ca\303\261al\000Casta\303\261al\000Pi\303\261al\000Fila San Rafael"
    "\000Isla Israel\000Cascabel\000La Isabel\000Villa Isabel\000Santa Isabel\000Monticel\000Redondel\000"
    "Cordel\000Vergel\000Punta de Riel\000Puerto Thiel\000San Gabriel\000Pilas de Canjel\000T\303\272nel"
    "\000Laurel\000Betel\000Plantel\000Hotel\000Playa San Miguel\000Bajo San Miguel\000T\303\251ruel\000A"
    "lto Espavel\000Quivel\000Marimil\000La Gu\303\241pil\000Once de Abril\000Veintisiete de Abril\000Nue"
    "vo Brasil\000Purisil\000Guaitil\000Cantil\000Cuajiniquil\000Jobo Civil\000Pueblo Civil\000Fox Hall"
    "\000G\303\274ell\000El Sol\000Ca\303\261ada del Sol\000Puerta del Sol\000Plaza del Sol\000Danza del "
    "Sol\000Monta\303\261a del Sol\000Valle del Sol\000Llanos del Sol\000Tr\303\251bol\000Santiago de Car"
    "acol\000Carmiol\000Pinol\000Guapinol\000Liverpool\000Chimirol\000Control\000Girasol\000Montesol\000P"
    "ocosol\000Bristol\000Lagos del Coyol\000Biscoyol\000Chassoul\000Quebrada Azul\000Piedra Azul\000Agua"
    " Azul\000Playa Azul\000Poza Azul\000Monta\303\261a Azul\000Valle Azul\000Monte Azul\000Llano Azul"
    "\000Cerro Azul\000Pozo Azul\000R\303\255o Azul\000Don Abraham\000Bearesem\000Fruta de Pan\000Pepe Lu"
    "jan\000El Plan\000Duan\000Bajo San Juan\000Alto San Juan\000Coopesanjuan\000La Virgen\000Goshen\000B"
    "remen\000El Carmen\000Alto del Carmen\000Boca Cuen\000Bajo Cuen\000Brooklin\000Lincoln\000Gonz\303"
    "\241lez Lahmann\000La Asuncion\000Evans Gordon Wilson\000Johnson\000Livingston\000Boston\000Bun\000G"
    "uti\303\251rrez Braun\000Jamaica Town\000Cana\303\241n\000Ticab\303\241n\000Anab\303\241n\000Michoac"
    "\303\241n\000Guayac\303\241n\000Volc\303\241n\000Cruce de Jord\303\241n\000San Juli\303\241n\000San "
    "Sebasti\303\241n\000Luj\303\241n\000Monte Gal\303\241n\000Laurel Gal\303\241n\000Gavil\303\241n\000Q"
    "uirim\303\241n\000San Hern\303\241n\000Cop\303\241n\000Tilar\303\241n\000Herr\303\241n\000Quesada Du"
    "r\303\241n\000Jorge Brenes Dur\303\241n\000Chumbur\303\241n\000Bat\303\241n\000Yucat\303\241n\000Tit"
    "\303\241n\000Moraz\303\241n\000El Ed\303\251n\000Nueva Jerusal\303\251n\000Bel\303\251n\000Boca Ur"
    "\303\251n\000Alto Ur\303\251n\000Mes\303\251n\000Kuib\303\255n\000Jard\303\255n\000Estaci\303\263n R"
    "ud\303\255n\000Canf\303\255n\000Guachipel\303\255n\000Viol\303\255n\000Berl\303\255n\000Tul\303\255n"
    "\000Capul\303\255n\000Jazm\303\255n\000Empalme Mo\303\255n\000San Valent\303\255n\000San Mart\303"
    "\255n\000Fort\303\255n\000San Agust\303\255n\000San Joaqu\303\255n\000Palo de Jab\303\263n\000Eslab"
    "\303\263n\000Ceib\303\263n\000Carb\303\263n\000Rinc\303\263n\000Pasc\303\263n\000Quebrad\303\263n"
    "\000Jos\303\251 Mar\303\255a Zeled\303\263n\000P\303\251rez Zeled\303\263n\000Bord\303\263n\000Juan "
    "de Le\303\263n\000Bajos de Le\303\263n\000Llano Le\303\263n\000Pedro Le\303\263n\000Sif\303\263n\000"
    "Arag\303\263n\000Peg\303\263n\000Obreg\303\263n\000Manch\303\263n\000Monte Si\303\263n\000Plantaci"
    "\303\263n\000Estaci\303\263n\000Asunci\303\263n\000Llanos de Concepci\303\263n\000Alto Concepci\303"
    "\263n\000La Uni\303\263n\000Bajos de la Uni\303\263n\000Reuni\303\263n\000Gorri\303\263n\000Mansi"
    "\303\263n\000Ilusi\303\263n\000Vegas de Caj\303\263n\000Laj\303\263n\000Alto del Moj\303\263n\000Res"
    "bal\303\263n\000Corral\303\263n\000Portal\303\263n\000Vuelta Tabl\303\263n\000Puerto Pil\303\263n"
    "\000Pabell\303\263n\000Mill\303\263n\000Villa Col\303\263n\000Nuevo Col\303\263n\000Am\303\263n\000C"
    "olonia San Ram\303\263n\000Boca Lim\303\263n\000Mata de Lim\303\263n\000Llano Lim\303\263n\000Alto L"
    "im\303\263n\000Tourn\303\263n\000Bar\303\263n\000Camar\303\263n\000Llamar\303\263n\000Hebr\303\263n"
    "\000Alto Calder\303\263n\000Bello Higuer\303\263n\000Paso Higuer\303\263n\000Boquer\303\263n\000Jor"
    "\303\263n\000Mocor\303\263n\000Garr\303\263n\000Chaparr\303\263n\000Cerr\303\263n\000Mes\303\263n"
    "\000Bols\303\263n\000Zapat\303\263n\000Marat\303\263n\000Alto Parrit\303\263n\000Plant\303\263n\000A"
    "lturas de Cot\303\263n\000Port\303\263n\000Pav\303\263n\000Play\303\263n\000Hoy\303\263n\000Sagrado "
    "Coraz\303\263n\000Reventaz\303\263n\000Trompez\303\263n\000Tropez\303\263n\000Poz\303\263n\000Luz"
    "\303\263n\000Bre\303\261\303\263n\000Verd\303\272n\000Calle Com\303\272n\000Bilbao\000Playa Cacao"
    "\000Llanada del Cacao\000Rinc\303\263n Cacao\000Bajo Cacao\000Alto Cacao\000Callao\000Carao\000Davao"
    "\000Paso Guabo\000Colonia Guayabo\000El Ceibo\000Bajo Ceibo\000Alto Ceibo\000Limbo\000Cachimbo\000Jo"
    "bo\000Garrobo\000San Paco\000R\303\255o Tabaco\000Tacaco\000M\303\263naco\000Boca Chocuaco\000Meco"
    "\000Isla Palo Seco\000Pueblo Seco\000Puerto Seco\000Finca Brazo Seco\000Cabeceras de R\303\255o Seco"
    "\000Ca\303\261o Seco\000Paloseco\000Hueco\000Mu\303\261eco\000Villa Lico\000Cuesta El Mico\000R\303"
    "\255o Tico\000Rancho Chico\000Orochico\000Chumico\000Abanico\000M\303\263nico\000Perico\000Burrico"
    "\000P\303\263rtico\000Arom\303\241tico\000Chirivico\000M\303\251xico\000Acapulco\000Punta Banco\000C"
    "abo Blanco\000Palo Blanco\000Astro Blanco\000San Miguel de R\303\255o Blanco\000San Ram\303\263n de "
    "R\303\255o Blanco\000Ca\303\261o Blanco\000Coliblanco\000Cariblanco\000Flamenco\000Maquenco\000Finca"
    " Cinco\000Cuarenta y Cinco\000Playa Coco\000El Coco\000Isla del Coco\000Choco\000Barco\000El Guarco"
    "\000Villas de Ayarco\000Lomas de Ayarco\000Izarco\000Vuelta de Jorco\000Lomas de Jorco\000Bajos de J"
    "orco\000R\303\255o Jorco\000Pedro Nolasco\000Damasco\000Unesco\000Tapesco\000Cerro Fresco\000San Ger"
    "ardo Viento Fresco\000Disco\000Pisco\000Lomas de San Francisco\000Aguacaliente o San Francisco\000Al"
    "to San Francisco\000Jalisco\000San Bosco\000San Juan Bosco\000Don Bosco\000Isla Bejuco\000Punta Beju"
    "co\000Cabeceras de R\303\255o Bejuco\000Tapezco\000Rinc\303\263n Orozco\000Dur\303\241n Picado\000Cl"
    "orito Picado\000R\303\255o Regado\000Sangregado\000Aguilar Machado\000Salado\000El Collado\000El Que"
    "mado\000Rancho Quemado\000Boca Ganado\000Quebrada Ganado\000Isla Venado\000Bajo Venado\000V\303\241z"
    "quez de Coronado\000Arado\000Colonia del Prado\000Anselmo Alvarado\000Barco Quebrado\000Dorado\000Ba"
    "jo Morado\000Barro Morado\000Veguitas de Colorado\000Barra del Colorado\000Estero Colorado\000Potrer"
    "o Cerrado\000Desjarretado\000Toledo\000Acevedo\000Bajo Escondido\000Pueblo Escondido\000Puerto Escon"
    "dido\000Los Guido\000Quebrada Nando\000Cerro Pando\000Obando\000San Fernando\000Puerto Lindo\000Tama"
    "rindo\000Barro Hondo\000Paso Hondo\000R\303\255o Hondo\000Monte Redondo\000Rancho Redondo\000Pueblo "
    "Redondo\000Cerro Redondo\000Rinc\303\263n Elizondo\000R\303\255o Segundo\000San Edmundo\000Bajo Raim"
    "undo\000Rinc\303\263n de Ricardo\000Cuesta Pichardo\000Puente Fajardo\000Gallardo\000San Bernardo"
    "\000Vuelta San Gerardo\000Zancudo\000Leo\000Rodeo\000Mata de Guineo\000Recreo\000Jateo\000San Mateo"
    "\000L\303\263pez Mateo\000Golfo\000Triunfo\000Alto Santiago\000Bajo Murci\303\251lago\000Rancho Rel"
    "\303\241mpago\000Alto Cartago\000Ca\303\261o Ciego\000San Diego\000Piedra de Fuego\000Piedras de Fue"
    "go\000Higo\000Alm\303\241cigo\000Hidalgo\000Mango\000Santo Domingo\000Bajo Congo\000Alto de Ochomogo"
    "\000Rancho Largo\000Hamburgo\000Cuesta Yugo\000Finca Ocho\000Finca Zona Ocho\000R\303\255o Macho\000"
    "Alto Camacho\000Cucaracho\000Quebracho\000Paso Ancho\000Concho\000Cerro Mocho\000Atrocho\000Caucho"
    "\000Caracucho\000Turbio\000Rodrigo Facio\000Bonifacio\000San Ignacio\000Topacio\000Lomas de San Panc"
    "racio\000Jardines del Beneficio\000Puerto Mauricio\000Rocas Amancio\000Colonia Silencio\000El Silenc"
    "io\000Bocas del R\303\255o Silencio\000Bajo Sucio\000Boca R\303\255o Sucio\000Estadio\000Campo Dos y"
    " Medio\000Ca\303\261o Seco Enmedio\000R\303\255o Enmedio\000Cerro Indio\000Alto Indio\000R\303\255o "
    "Incendio\000Colonia Colegio\000Refugio\000San Cecilio\000San Emilio\000Volio\000Veinticinco de Julio"
    "\000Treminio\000Manuel Antonio\000Alto San Antonio\000Tres de Junio\000Hogar Propio\000La Carpio\000"
    "Escorpio\000San Macario\000Precario\000Balneario\000Campanario\000Seminario\000Rosario\000Sagitario"
    "\000Ontario\000Calvario\000Vegas de Imperio\000Calle Cementerio\000Delirio\000Montelirio\000Martirio"
    "\000Laborio\000Tenorio\000Crematorio\000Oratorio\000Laboratorio\000Sipurio\000Sitio\000Domingo Savio"
    "\000El Bajo\000Grifo Bajo\000Carrillos Bajo\000Calle Tajo\000Chirraca Abajo\000Tortuga Abajo\000Chim"
    "urria Abajo\000Jabilla Abajo\000Bonilla Abajo\000Savegre Abajo\000Pataste Abajo\000San Rafael Abajo"
    "\000Santa Isabel Abajo\000San Juan Abajo\000Sam\303\251n Abajo\000Ronr\303\263n Abajo\000Ca\303\261o"
    " Seco Abajo\000San Antonio Abajo\000Junquillo Abajo\000R\303\255o Chiquito Abajo\000Mesetas Abajo"
    "\000Ca\303\261ales Abajo\000Oricuajo\000Arenal Viejo\000Limonal Viejo\000General Viejo\000Higuer\303"
    "\263n Viejo\000San Pablo Viejo\000Pueblo Viejo\000Hatillo Viejo\000San Pedro Viejo\000Rastro Viejo"
    "\000Hato Viejo\000San Alberto Viejo\000Puerto Viejo\000R\303\255o Viejo\000R\303\255o Ca\303\261as V"
    "iejo\000Pejeviejo\000San Alejo\000Alto Alejo\000Camejo\000Bermejo\000R\303\255o Conejo\000Botijo\000"
    "Finca Cortijo\000Franjo\000Boca Naranjo\000Fila Naranjo\000Junta Naranjo\000Legua de Naranjo\000Alto"
    " Naranjo\000R\303\255o Naranjo\000Vegas de Abrojo\000Altos del Brujo\000Bajo Brujo\000Cerro Brujo"
    "\000Isla S\303\241balo\000Bajo S\303\241balo\000Alto S\303\241balo\000B\303\272falo\000Playa Matapal"
    "o\000Chirvalo\000San Pablo\000Cuesta El Diablo\000Pueblo\000Ciudad del Cielo\000Terciopelo\000Consue"
    "lo\000Silo\000Asilo\000Cola de Gallo\000Pata de Gallo\000El Gallo\000Isla Caballo\000Rossiter Carbal"
    "lo\000Cantagallo\000Colegallo\000Monte Bello\000Jabillo\000Plancillo\000Jaboncillo\000Cordoncillo"
    "\000Playa Cuchillo\000Bajillo\000Naranjillo\000Robalillo\000Guacalillo\000Huacalillo\000Charcalillo"
    "\000Guinealillo\000Corralillo\000Patalillo\000Candelillo\000Platanillo\000San Juanillo\000Manzanillo"
    "\000Molenillo\000Cocal Amarillo\000Cielo Amarillo\000Brisas del Toro Amarillo\000San Pedrillo\000Pue"
    "rto Carrillo\000Cerrillo\000Encierrillo\000Entierrillo\000Salitrillo\000Alto Murillo\000Hatillo\000S"
    "altillo\000Calientillo\000Conventillo\000Achiotillo\000Torrotillo\000Pizotillo\000Playa Lagartillo"
    "\000Cuesta Portillo\000Florencio del Castillo\000Bajo Castillo\000Patastillo\000Fanguillo\000Estanqu"
    "illo\000Roquillo\000Charquillo\000Cocobolo\000Nacascolo\000Apolo\000Bartolo\000Montecarlo\000Tri\303"
    "\241ngulo\000R\303\263tulo\000Nuestro Amo\000P\303\251njamo\000Alamo\000P\303\241ramo\000Gu\303\241c"
    "imo\000San Jer\303\263nimo\000Estocolmo\000Monte Romo\000Plomo\000Alto Palomo\000Palermo\000San Guil"
    "lermo\000Jormo\000Curva del Humo\000Alto Humo\000Puerto Humo\000Finca Uno\000Viquilla Uno\000Indiana"
    " Uno\000Calle Uno\000El Uno\000San Juan Uno\000Villas del Mar Uno\000L\303\255bano\000C\303\263bano"
    "\000Americano\000Chiricano\000Fray Casiano\000El Llano\000Paso Llano\000Alto Llano\000Porto Llano"
    "\000Milano\000Bajo Solano\000Los Solano\000R\303\255o Banano\000Palo Campano\000Hispano\000Belgrano"
    "\000Mata de Platano\000San Cayetano\000Boca del Pantano\000Montano\000Mata de Pl\303\241tano\000Ruan"
    "o\000Zoncuano\000Cacaovano\000Manzano\000R\303\255o Magdaleno\000Nazareno\000R\303\255o Sereno\000Pu"
    "erto Moreno\000Centeno\000Del Pino\000Tulita Sandino\000San Bernardino\000Chino\000Molino\000Alpino"
    "\000San Marino\000Don Victorino\000Latino\000Mono\000Calle Gobierno\000Infierno\000Torno\000Saturno"
    "\000Siglo Veintiuno\000Oreamuno\000Bruno\000Durazno\000Boca Sah\303\255no\000Alto Sah\303\255no\000C"
    "epo\000El Campo\000Obispo\000Cuenca de Oro\000Castilla de Oro\000Vista de Oro\000R\303\255o de Oro"
    "\000Montes de Oro\000Campos de Oro\000Brazos de Oro\000R\303\255o Oro\000Faro\000J\303\255caro\000Al"
    "faro\000Vegas de R\303\255o Claro\000Monteclaro\000Bajo J\303\251naro\000El Amparo\000San Jos\303"
    "\251 del Amparo\000Alc\303\241ntaro\000G\303\274izaro\000San L\303\241zaro\000Cen\303\255zaro\000G"
    "\303\274\303\255zaro\000Cabro\000Sacro\000R\303\255o Cedro\000Vuelta San Pedro\000Calle Don Pedro"
    "\000San Isidro\000San Alejandro\000Olla Cero\000Alto Raicero\000Cenicero\000Zarcero\000Crucero\000Em"
    "barcadero\000Mochadero\000Saladero\000Bebedero\000Ponedero\000Hervidero\000Caballero\000Semillero"
    "\000Astillero\000Romero\000Sabanero\000Cambronero\000N\303\255spero\000Sombrero\000Pedrero\000Terrer"
    "o\000Calle Guerrero\000Monte Potrero\000Montero\000Lagartero\000Estero\000Jilguero\000Vegas de Tortu"
    "guero\000Ca\303\261o Chiquero\000Barquero\000Milagro\000Puente Negro\000Monte Negro\000Tronco Negro"
    "\000Bajo Negro\000Barro Negro\000R\303\255o Negro\000Ca\303\261o Negro\000Montenegro\000Peligro\000E"
    "l Cairo\000Retiro\000Colonia del Toro\000Cuesta del Toro\000Uni\303\263n del Toro\000Lomas del Toro"
    "\000Bajos del Toro\000F\303\263sforo\000Lomas de Ocloro\000Poroporo\000El Barro\000Chamarro\000Los N"
    "avarro\000Cerro\000Quebrada del Fierro\000Destierro\000Pirro\000Atirro\000Altos del Socorro\000Perpe"
    "tuo Socorro\000Chorro\000Chamorro\000Papaturro\000Campo Cuatro\000Kil\303\263metro\000Le\303\263n XI"
    "II Centro\000La Ceiba Centro\000San Rafael Arriba Centro\000Turrialba Centro\000Tarbaca Centro\000Va"
    "ra Blanca Centro\000Tierra Blanca Centro\000Barranca Centro\000Uruca Centro\000Boruca Centro\000Ques"
    "ada Centro\000Florida Centro\000Quebrada Honda Centro\000Sabana Redonda Centro\000Hojancha Centro"
    "\000Venecia Centro\000Grecia Centro\000Florencia Centro\000Tabarcia Centro\000Heredia Centro\000Fila"
    "delfia Centro\000Santa Eulalia Centro\000Santa Cecilia Centro\000Germania Centro\000Candelaria Centr"
    "o\000Liberia Centro\000Hacienda Vieja Centro\000La Granja Centro\000Upala Centro\000Alajuela Centro"
    "\000Granadilla Centro\000Quebradilla Centro\000Sabanilla Centro\000Guadalupe o Arenilla Centro\000Si"
    "xaola Centro\000Gu\303\241cima Centro\000Colima Centro\000Santa Ana Centro\000Roxana Centro\000Carta"
    "gena Centro\000Aguabuena Centro\000Matina Centro\000Orotina Centro\000Macacona Centro\000Carmona Cen"
    "tro\000Laguna Centro\000La Fortuna Centro\000Santa B\303\241rbara Centro\000S\303\241mara Centro\000"
    "Puente de Piedra Centro\000La Ribera Centro\000La Palmera Centro\000Paquera Centro\000La Tigra Centr"
    "o\000Palmira Centro\000Katira Centro\000Pocora Centro\000Tronadora Centro\000Pandora Centro\000Santa"
    " Rosa Centro\000Santa Rita Centro\000Alajuelita Centro\000La Garita Centro\000Chacarita Centro\000Ca"
    "ndelarita Centro\000Bel\303\251n de Nosarita Centro\000Parrita Centro\000Cahuita Centro\000Peralta C"
    "entro\000La Cuesta Centro\000Buena Vista Centro\000Buenavista Centro\000Patio de Agua Centro\000Bija"
    "gua Centro\000Legua Centro\000Barva Centro\000Pitahaya Centro\000Nicoya Centro\000La Suiza Centro"
    "\000Zaragoza Centro\000Esparza Centro\000Santa Luc\303\255a Centro\000Santa Mar\303\255a Centro\000J"
    "es\303\272s Mar\303\255a Centro\000Alegr\303\255a Centro\000San Jos\303\251 de la Monta\303\261a Cen"
    "tro\000Quebrada Grande Centro\000San Juan Grande Centro\000Llano Grande Centro\000Potrero Grande Cen"
    "tro\000El Roble Centro\000San Felipe Centro\000Sierpe Centro\000Guadalupe Centro\000Dulce Nombre Cen"
    "tro\000Tempate Centro\000El Mastate Centro\000Canalete Centro\000San Vicente Centro\000Anselmo Llore"
    "nte Centro\000Desmonte Centro\000Zapote Centro\000San Jos\303\251 o Pizote Centro\000Piedades Norte "
    "Centro\000Sarch\303\255 Norte Centro\000Tucurrique Centro\000San Roque Centro\000Pejibaye Centro\000"
    "Cariari Centro\000Bribri Centro\000Tobosi Centro\000Orosi Centro\000Escobal Centro\000Palmichal Cent"
    "ro\000Cangrejal Centro\000Guacimal Centro\000Arenal Centro\000Sardinal Centro\000San Isidro de el Ge"
    "neral Centro\000Purral Centro\000Salitral Centro\000Pital Centro\000Zapotal Centro\000Carrizal Centr"
    "o\000Porozal Centro\000San Rafael Centro\000Santa Isabel Centro\000San Gabriel Centro\000Laurel Cent"
    "ro\000San Miguel Centro\000Veintisiete de Abril Centro\000Brasil Centro\000Guaitil Centro\000R\303"
    "\255o Azul Centro\000San Juan Centro\000La Virgen Centro\000Carmen Centro\000La Asuncion Centro\000C"
    "oncepcion Centro\000Volc\303\241n Centro\000San Sebasti\303\241n Centro\000Tilar\303\241n Centro\000"
    "Bat\303\241n Centro\000Bel\303\251n Centro\000Jard\303\255n Centro\000San Joaqu\303\255n Centro\000C"
    "oncepci\303\263n Centro\000La Uni\303\263n Centro\000Mansi\303\263n Centro\000Caj\303\263n Centro"
    "\000Col\303\263n Centro\000San Ram\303\263n Centro\000Lim\303\263n Centro\000Bols\303\263n Centro"
    "\000Guayabo Centro\000Vuelta de Jorco Centro\000Aguacaliente o San Francisco Centro\000Bejuco Centro"
    "\000Tapezco Centro\000Venado Centro\000Colorado Centro\000Potrero Cerrado Centro\000Los Guido Centro"
    "\000Rancho Redondo Centro\000R\303\255o Segundo Centro\000San Mateo Centro\000Santiago Centro\000San"
    " Diego Centro\000Santo Domingo Centro\000San Ignacio Centro\000Volio Centro\000San Antonio Centro"
    "\000El Rosario Centro\000San Rafael Abajo Centro\000Puerto Viejo Centro\000R\303\255o Naranjo Centro"
    "\000San Pablo Centro\000Corralillo Centro\000Manzanillo Centro\000Puerto Carrillo Centro\000Hatillo "
    "Centro\000Gu\303\241cimo Centro\000San Jer\303\263nimo Centro\000Monte Romo Centro\000L\303\255bano "
    "Centro\000C\303\263bano Centro\000Mata de Pl\303\241tano Centro\000El Amparo Centro\000San Pedro Cen"
    "tro\000San Isidro Centro\000Zarcero Centro\000Bebedero Centro\000Ca\303\261o Negro Centro\000El Cair"
    "o Centro\000San Franciso Centro\000Para\303\255so Centro\000San Vito Centro\000Paracito Centro\000Sa"
    "n Josecito Centro\000Limoncito Centro\000Golfito Centro\000Naranjito Centro\000Sabalito Centro\000Pa"
    "cuarito Centro\000Grifo Alto Centro\000Lepanto Centro\000R\303\255o Cuarto Centro\000San Lorenzo Cen"
    "tro\000El Tejar Centro\000Coyolar Centro\000Miramar Centro\000Tambor Centro\000Labrador Centro\000Pi"
    "edades Sur Centro\000Mercedes Sur Centro\000Sarch\303\255 Sur Centro\000Cirr\303\255 Sur Centro\000H"
    "uacas Centro\000Piedras Blancas Centro\000Aguas Zarcas Centro\000Delicias Centro\000Gravilias Centro"
    "\000Pilas Centro\000Sabanillas Centro\000Esquipulas Centro\000Puntarenas Centro\000Tierras Morenas C"
    "entro\000Atenas Centro\000Cinco Esquinas Centro\000Barbacoas Centro\000Canoas Centro\000Piedras Negr"
    "as Centro\000Las Horquetas Centro\000Las Juntas Centro\000Pavas Centro\000Rivas Centro\000Pacayas Ce"
    "ntro\000Ca\303\261as Centro\000Juan Vi\303\261as Centro\000Bagaces Centro\000Ca\303\261as Dulces Cen"
    "tro\000Piedades Centro\000Capellades Centro\000Mercedes Centro\000\303\201ngeles Centro\000Frailes C"
    "entro\000Los Chiles Centro\000Chomes Centro\000Pavones Centro\000Tacares Centro\000Turr\303\272cares"
    " Centro\000Palmares Centro\000Picagres Centro\000Buenos Aires Centro\000Siquirres Centro\000Tirrases"
    " Centro\000Cipreses Centro\000Cervantes Centro\000Jaris Centro\000San Luis Centro\000Tuis Centro\000"
    "Tres Equis Centro\000Calle Blancos Centro\000San Marcos Centro\000Desamparados Centro\000San Juan de"
    " Dios Centro\000Esterillos Centro\000Salitrillos Centro\000San Carlos Centro\000Quepos Centro\000Des"
    "amparaditos Centro\000Palmitos Centro\000Pozos Centro\000Tres R\303\255os Centro\000San Francisco de"
    " Dos R\303\255os Centro\000Santo Tom\303\241s Centro\000San Andr\303\251s Centro\000Puerto Cort\303"
    "\251s Centro\000Ip\303\255s Centro\000Dulce Nombre de Jes\303\272s Centro\000Curridabat Centro\000Co"
    "t Centro\000Copey Centro\000Monterrey Centro\000Neily Centro\000S\303\241nchez Centro\000Puerto Jim"
    "\303\251nez Centro\000R\303\255o Jim\303\251nez Centro\000La Cruz Centro\000Santa Cruz Centro\000Pat"
    "arr\303\241 Centro\000San Jos\303\251 Centro\000Cach\303\255 Centro\000Aserr\303\255 Centro\000Quiti"
    "rris\303\255 Centro\000Jac\303\263 Centro\000Matamb\303\272 Centro\000Escaz\303\272 Centro\000Carvaj"
    "al Castro\000Facio Castro\000Alto Castro\000Le\303\263n Cort\303\251s Castro\000Rastro\000Muro\000Ce"
    "rro Oscuro\000Futuro\000Paso\000El Progreso\000Medio Queso\000Paraiso\000San Franciso\000Treviso\000"
    "Balso\000Descanso\000Alto Alonso\000Pedregoso\000Valle Hermoso\000Guatuso\000Para\303\255so\000Gato"
    "\000Pato\000Colepato\000Pito\000San Vito\000Garabito\000Guayabito\000Bambito\000Panamacito\000Paraci"
    "to\000San Josecito\000Montecito\000Aranjuecito\000Volcancito\000Quirimancito\000San Juancito\000Moll"
    "ejoncito\000Mojoncito\000Milloncito\000Alto Limoncito\000Pocito\000Barucito\000Puerto Coloradito\000"
    "Obandito\000Rodeito\000Golfito\000Mongito\000Cachito\000Conchito\000Ojochito\000Naranjito\000Puerto "
    "Palito\000Sabalito\000Guacalito\000Dominicalito\000Cocalito\000Naranjalito\000Guarumalito\000Jabonal"
    "ito\000Chapernalito\000Moralito\000Pitalito\000Caimitalito\000Bijagualito\000Corozalito\000San Pabli"
    "to\000Canjelito\000San Miguelito\000Brasilito\000Gallito\000Caballito\000Arbolito\000Coyolito\000Cai"
    "mito\000San Benito\000Remolinito\000Valle Bonito\000Llano Bonito\000R\303\255o Bonito\000Jicarito"
    "\000Pocarito\000Altos de Pacuarito\000San Pedrito\000Saladerito\000Esterito\000Tigrito\000Torito\000"
    "Navarrito\000Cerrito\000Chamorrito\000Burrito\000Pasito\000Poasito\000Alto Birrisito\000Bajo Mansito"
    "\000Pedregosito\000Chirriposito\000Tausito\000Guatusito\000R\303\255o Quito\000Higuito\000Matambugui"
    "to\000San Juan Chiquito\000Rinc\303\263n Chiquito\000Potrero Chiquito\000R\303\255o Chiquito\000Coqu"
    "ito\000Cuervito\000Riyito\000Puente Alto\000Monte Alto\000El Alto\000Grifo Alto\000Tajo Alto\000Carr"
    "illos Alto\000R\303\255o Salto\000Roblealto\000Canto\000Danto\000Pueblo Santo\000Esp\303\255ritu San"
    "to\000Calicanto\000Alto Encanto\000Lepanto\000Tapa Viento\000Pimiento\000Renacimiento\000Sarmiento"
    "\000Sacramento\000Convento\000Pinto\000Tiocinto\000Gallopinto\000Laberinto\000Corinto\000Metaponto"
    "\000Fortuna de Coto\000Esperanza de Coto\000Uni\303\263n de Coto\000Bajo de Coto\000Flor de Coto\000"
    "Bajos de Coto\000R\303\255o Coto\000Loto\000Eucalipto\000Paso Lagarto\000R\303\255o Cuarto\000San Hu"
    "mberto\000Huerto\000Aeropuerto\000Repasto\000Santo Cristo\000Montecristo\000Quince de Agosto\000Barr"
    "io Nuevo\000Pueblo Nuevo\000Hatillo Nuevo\000Castillo Nuevo\000San Alberto Nuevo\000Puerto Nuevo\000"
    "R\303\255o Nuevo\000R\303\255o Ca\303\261as Nuevo\000Barrionuevo\000Chivo\000Olivo\000Bajo Calvo\000"
    "Primero de Mayo\000Rayo\000Papagayo\000Surayo\000Hoyo\000Soncoyo\000Quebrada Arroyo\000El Arroyo\000"
    "Leiva Urcuyo\000Chucuyo\000Tablazo\000Alto Ca\303\261azo\000Rojizo\000Cenizo\000Erizo\000San Lorenzo"
    "\000Pozo\000Corozo\000Doce de Marzo\000Primero de Marzo\000R\303\255o Marzo\000Colonia del R\303\255"
    "o\000Punta del R\303\255o\000Valle del R\303\255o\000Condado del R\303\255o\000Pinar del R\303\255o"
    "\000Lomas del R\303\255o\000Boh\303\255o\000Bajos de Ar\303\255o\000Cerro Fr\303\255o\000R\303\255o "
    "Fr\303\255o\000Sabor\303\255o\000Bur\303\255o\000Cur\303\255o\000Alto del Desenga\303\261o\000Casta"
    "\303\261o\000Villa Brice\303\261o\000Pati\303\261o\000Cuesta Madro\303\261o\000Tunel Camp\000Villabo"
    "nita Vista Mar\000Vista de Mar\000Vista Al Mar\000Cam\303\255bar\000San Jos\303\251 Cabecar\000Pan d"
    "e Az\303\272car\000Pil\303\263n de Az\303\272car\000Hogar\000Azahar\000El Tejar\000San Antonio del T"
    "ejar\000Bajo Tejar\000Copalar\000Corralar\000Roblar\000Alto Laurelar\000Navajuelar\000Espavelar\000M"
    "anglar\000Pilar\000Aguilar\000Solar\000Frijolar\000Coyolar\000Alto Jaular\000Lindamar\000Carrizal de"
    " Bajamar\000San Juan de Miramar\000Alto Miramar\000Montelimar\000Solimar\000Palmar\000Platanar\000Co"
    "lmenar\000Fila Pinar\000Lomas del Pinar\000Sipar\000Llanuras del Gaspar\000Balsar\000Colonia Huetar"
    "\000Gibraltar\000Vallevistar\000Juntas de Pacuar\000Bajos de Pacuar\000Tovar\000Bol\303\255var\000Ca"
    "stro Salazar\000Badilla Alpizar\000Rinc\303\263n de Alp\303\255zar\000Santander\000Fletcher\000Pitti"
    "er\000Culpeper\000Vuelta de Kooper\000Kopper\000Uni\303\263n Ferrer\000Alma M\303\241ter\000Meseguer"
    "\000Namuwakir\000Porvenir\000Ca\303\261o Tambor\000Roncador\000El Volador\000Calle Amador\000Bajo Am"
    "ador\000Sonador\000Labrador\000Quebrador\000Alto Mirador\000San Salvador\000Corredor\000Villa Flor"
    "\000Nueva Flor\000Milflor\000Vapor\000Buen Pastor\000Arancibia Sur\000Filadelfia Sur\000Granadilla S"
    "ur\000Verbena Sur\000Barra de Matina Sur\000Santa Clara Sur\000Pocora Sur\000Favorita Sur\000Esperan"
    "za Sur\000El Sur\000San Crist\303\263bal Sur\000Sardinal Sur\000Cedral Sur\000Tuetal Sur\000Palmital"
    " Sur\000San Rafael Sur\000Colonia del Sur\000Estrella del Sur\000Valle del Sur\000Uni\303\263n del S"
    "ur\000San Miguel Sur\000Caracol Sur\000San Juan Sur\000San Mart\303\255n Sur\000Misi\303\263n Sur"
    "\000Play\303\263n Sur\000Jilguero Sur\000Bananito Sur\000Llano Bonito Sur\000Torito Sur\000Encanto S"
    "ur\000Palmar Sur\000Piedades Sur\000Mercedes Sur\000\303\201ngeles Sur\000Yoses Sur\000Santa Gertrud"
    "is Sur\000Cartagos Sur\000San Jos\303\251 Sur\000Sarch\303\255 Sur\000Cirr\303\255 Sur\000Cas\000Yas"
    "\000Itabas\000Guabas\000Alto Tumbas\000Islas Cubas\000Hamacas\000Canacas\000Caracas\000Cuacas\000Alt"
    "o Huacas\000Micas\000Las Am\303\251ricas\000Pilas Blancas\000Piedras Blancas\000Pe\303\261as Blancas"
    "\000Bruncas\000Cuatro Bocas\000Dos Bocas\000Las Rocas\000Aguas Zarcas\000Dos Cercas\000Aguas Frescas"
    "\000Colonia San Lucas\000Las Hadas\000Cascadas\000Ramadas\000Gradas\000Tres Quebradas\000Altamiradas"
    "\000Correntadas\000Ca\303\261adas\000Alamedas\000Vuelta Ruedas\000Naciones Unidas\000Esmeraldas\000A"
    "guas Lindas\000Alto Vindas\000Rotondas\000Bajo Cerdas\000Ca\303\261as Gordas\000Punta Judas\000Picud"
    "as\000Las Orqu\303\255deas\000Az\303\241leas\000Guineas\000Ci\303\251nagas\000Las Vegas\000Bodegas"
    "\000Alto Villegas\000Banegas\000Vanegas\000Rinc\303\263n Venegas\000Paso Vigas\000Pangas\000Domingas"
    "\000Calle Vargas\000Carlos Vargas\000Acacias\000Delicias\000Calle Caricias\000Tiricias\000Camelias"
    "\000Gravilias\000Magnolias\000Armenias\000Colonias\000Rinc\303\263n de Arias\000Bajo Arias\000Asturi"
    "as\000Hortensias\000San Juan de Lajas\000Paso Lajas\000Alhajas\000Cuesta Rojas\000Abelardo Rojas\000"
    "C\303\241talo Rojas\000Trojas\000Agujas\000San Blas\000Rinc\303\263n de Salas\000Puente Salas\000Alt"
    "o Salas\000Tablas\000Cabo Velas\000Parcelas\000Gemelas\000Bajo Varelas\000Bruselas\000Ciruelas\000Pl"
    "ayuelas\000Lilas\000Bajo Pilas\000Alto Pilas\000Calle Fallas\000Farallas\000Once Estrellas\000Villas"
    "\000Quebradillas\000Sabanillas\000Hornillas\000Lagunillas\000Piedras Amarillas\000Higuerillas\000Tra"
    "nquerillas\000Casillas\000Cuestillas\000Bolas\000Tres Perlas\000Esquipulas\000Camas\000Isla Damas"
    "\000Calle Damas\000Estero Damas\000R\303\255o Damas\000Dos Ramas\000San Dimas\000Limas\000\303\201ni"
    "mas\000Alto Palmas\000Las Palmas\000Lomas\000Bajo Palomas\000Alto Palomas\000Brumas\000Bajo Lanas"
    "\000Sabanas\000Cantarranas\000Ventanas\000Cien Manzanas\000Bajo C\303\241rdenas\000Finca Puntarenas"
    "\000Colonia Puntarenas\000Tierras Morenas\000Atenas\000Bajo Minas\000Cuatro Reinas\000Hacienda Salin"
    "as\000Catalinas\000Siete Colinas\000Tres Colinas\000Cortinas\000Cinco Esquinas\000Cuatro Esquinas"
    "\000Tres Esquinas\000Bajo M\303\241quinas\000Lagunas\000Alto Barbacoas\000Paso Canoas\000Lapas\000Mi"
    "lpas\000Pampas\000Chinampas\000Taras\000Bajo Claras\000Aguas Claras\000L\303\241mparas\000R\303\255o"
    " Piedras\000Tres Piedras\000Cabeceras\000Praderas\000Ajuntaderas\000Banderas\000Escaleras\000Palmera"
    "s\000Carreras\000Chorreras\000Piedras Negras\000Tierras Negras\000Aguas Negras\000Palmiras\000Loras"
    "\000Bajo Moras\000Pastoras\000Porras\000Las Letras\000Honduras\000Cerro Mesas\000Alto de las Mesas"
    "\000Bajo Brisas\000Alto Brisas\000Las Brisas\000Luisas\000Valle de las Rosas\000Tiquirusas\000Aguas "
    "Gatas\000Bajo Latas\000Cataratas\000Caletas\000Violetas\000Las Horquetas\000Calabacitas\000Florecita"
    "s\000Crucitas\000Agujitas\000Damas Israelitas\000Playuelitas\000Las Palmitas\000Lomitas\000Puntareni"
    "tas\000Salinitas\000Bajo Bonitas\000Floritas\000Casitas\000Mesitas\000Huaquitas\000Finquitas\000Cabe"
    "ceras de Aguas Zarquitas\000Pacayitas\000Playitas\000Ca\303\261itas\000Monta\303\261itas\000Quebrada"
    " Vueltas\000R\303\255o Vueltas\000Dantas\000Mantas\000Las Juntas\000Gaviotas\000Dos Aguas\000Alto Pa"
    "vas\000Las Pavas\000C\303\263ncavas\000Cuevas\000Rivas\000Pacayas\000Playas\000Joyas\000Cabezas\000M"
    "ellizas\000Bajo Esperanzas\000Trenzas\000Pozas\000Aguas Fr\303\255as\000Las Mar\303\255as\000Las Tre"
    "s Mar\303\255as\000Bajos Mat\303\255as\000Cabeceras de Ca\303\261as\000Moreno Ca\303\261as\000Las Ca"
    "\303\261as\000Caba\303\261as\000Pe\303\261as\000Bre\303\261as\000Juan Vi\303\261as\000Campi\303\261a"
    "s\000Fields\000Las Nubes\000Bagaces\000Codornices\000Ca\303\261as Dulces\000Nances\000Sauces\000Cuat"
    "ro Cruces\000Las Cruces\000Bajo Los Cruces\000Piedades\000Novedades\000Verdades\000Capellades\000Pl"
    "\303\251yades\000Ana Mercedes\000Las Mercedes\000Salsipuedes\000Bernardo Benavides\000Andes\000Duend"
    "es\000Calle Valverdes\000Bajo Lourdes\000Santa Eduviges\000Los Monges\000Malinches\000Aries\000Junca"
    "les\000Nogales\000Helechales\000Thiales\000Naranjales\000Rastrojales\000Estrellales\000Los Canales"
    "\000Panales\000Manzanales\000Tronconales\000Gamonales\000Pedernales\000Jirales\000Corales\000Punta M"
    "orales\000Rinc\303\263n Morales\000Corrales\000Salitrales\000Tres Rosales\000Cooperrosales\000Zacata"
    "les\000Coquitales\000Bajo Chontales\000Tacotales\000Bijaguales\000Pacayales\000Polvazales\000Carriza"
    "les\000Robles\000Punta Cocles\000Calle \303\201ngeles\000Llano \303\201ngeles\000Alto \303\201ngeles"
    "\000Sector \303\201ngeles\000Los \303\201ngeles\000Parajeles\000Laureles\000Claveles\000Espaveles"
    "\000Calle Liles\000Frailes\000Boca Los Chiles\000Gu\303\241piles\000Pretiles\000Versalles\000Miraval"
    "les\000Telles\000\303\201rboles\000T\303\241rcoles\000N\303\241poles\000Faroles\000Shiroles\000Coyol"
    "es\000Lomas Azules\000Campos Azules\000Chomes\000Fraijanes\000Los Planes\000Gavilanes\000Magallanes"
    "\000Llano Brenes\000Los Jardines\000Guachipelines\000Jazmines\000Bajo Chacones\000Horcones\000Leones"
    "\000Guiones\000Mollejones\000Pilones\000Camones\000Limones\000Camarones\000Esterones\000Higuerones"
    "\000Cimarrones\000Portones\000Alto Pavones\000Playones\000Cuesta Mara\303\261ones\000Torunes\000Alpe"
    "s\000Finca Tres\000Indiana Tres\000Roxana Tres\000El Tres\000Campo Tres\000Tres y Tres\000Turrubares"
    "\000Tacares\000Villacares\000Pocares\000C\303\272scares\000Turr\303\272cares\000Almendares\000T\303"
    "\272fares\000Dos de Abangares\000Aquiares\000Siquiares\000Bajo Tejares\000Pilares\000Aguilares\000So"
    "lares\000Palmares\000Platanares\000Pinares\000Huetares\000Cumbres\000Isla Pobres\000Cucubres\000Fila"
    " Surubres\000Tusubres\000Londres\000P\303\263geres\000Ticufres\000Paso Agres\000Picagres\000Pigres"
    "\000Chirogres\000Corrogres\000Cuesta Buenos Aires\000Alto Buenos Aires\000Bajo Paires\000Bajo Chires"
    "\000Purires\000Tiquires\000Tabores\000Contadores\000Corredores\000Hondores\000Autofores\000Ciudadela"
    " Flores\000Daniel Flores\000Paso Flores\000Las Flores\000Iglesias Flores\000Miraflores\000Villa Dolo"
    "res\000Castores\000Siquirres\000Bajos del Torres\000Tures\000Oses\000Tirrases\000Targuases\000Freses"
    "\000Cipreses\000Ulises\000Los Luises\000Yoses\000Bajo Dioses\000Poroses\000Bajo Piuses\000Estudiante"
    "s\000Diamantes\000Barrantes\000Cervantes\000Inocentes\000Presidentes\000Fuentes\000Miramontes\000Mac"
    "homontes\000Orontes\000Horizontes\000Pochotes\000Islotes\000Cambutes\000Diques\000Criques\000Tanques"
    "\000Rinc\303\263n Chaves\000Nieves\000Pe\303\261\303\263n de Tivives\000Bajo Reyes\000Hacienda Los R"
    "eyes\000Villa Los Reyes\000Alto Los Reyes\000Xirinachs\000Santa Gertrudis\000Calle Seis\000El Seis"
    "\000Noventa y Seis\000Santa Eduvigis\000Brusmalis\000Metr\303\263polis\000Tacanis\000Iroquois\000Bri"
    "s\000Zona Gris\000Rosa Iris\000Do\303\261a Iris\000Arco Iris\000Jaris\000Damaris\000Coris\000Cutris"
    "\000Oasis\000O\303\241sis\000Ca\303\261o San Luis\000Tuis\000Tres Equis\000El G\303\274is\000Villa M"
    "ills\000Finca Dos\000Viquilla Dos\000Indiana Dos\000El Dos\000Campo Dos\000Villas del Mar Dos\000Esq"
    "uipulas Dos\000Guabos\000Los Guayabos\000Jobos\000Lobos\000Colonia Villalobos\000Huecos\000Pericos"
    "\000Calle Blancos\000Arcos\000Rinc\303\263n San Marcos\000Paso Marcos\000Charcos\000Chucos\000Ahogad"
    "os\000Mojados\000Cuesta Quemados\000Venados\000Desamparados\000Dorados\000Arrepentidos\000Los Lagos"
    "\000Bajo Cartagos\000Centro de Amigos\000Boca Tres Amigos\000Seis Amigos\000Dos Amigos\000Los Amigos"
    "\000Mangos\000Alto Los Mogos\000Pargos\000Bajo Burgos\000Ranchos\000Chanchos\000Alto Gracias A Dios"
    "\000Madre de Dios\000San Juan de Dios\000Pueblo de Dios\000Vega de R\303\255o Palacios\000Villapalac"
    "ios\000Bajo Indios\000Paso Indios\000Colegios\000Geranios\000Vuelta Millonarios\000Jardines Universi"
    "tarios\000Naranjos Agrios\000Los Lirios\000Sitios\000Bajos\000Bajo Tajos\000Cangrejos\000Calle Naran"
    "jos\000Calle Los Naranjos\000Esperanza de S\303\241balos\000Los Abuelos\000Silos\000Jabillos\000Guay"
    "abillos\000Tejarcillos\000Huacalillos\000Charcalillos\000Bolillos\000Potrerillos\000Carrillos\000Cer"
    "rillos\000Salitrillos\000Patillos\000Charquillos\000Manolos\000Boca de San Carlos\000Muelle de San C"
    "arlos\000Don Carlos\000Alamos\000Gu\303\241cimos\000Llanos\000Solanos\000Matasanos\000Manzanos\000Ca"
    "marenos\000Morenos\000Dos Pinos\000Los Pinos\000Bajo Molinos\000Torremolinos\000Espinos\000Bajo Anon"
    "os\000Vuelta Cabo de Hornos\000Tornos\000Quepos\000Alto Campos\000Costa de P\303\241jaros\000Cuadros"
    "\000Isla Cedros\000Valle Cedros\000Bajo Cedros\000Almendros\000Raiceros\000Lavaderos\000Linderos\000"
    "Terreros\000Chiqueros\000Corrales Negros\000Palos Negros\000Loros\000Casilda Matamoros\000Angostura "
    "Matamoros\000Cerros\000Chorros\000Filtros\000Tauros\000Pocitos\000Corocitos\000Saucitos\000Desampara"
    "ditos\000Ranchitos\000Tarcolitos\000Caimitos\000Palmitos\000Turrubaritos\000Abangaritos\000Negritos"
    "\000Tiquiritos\000Cerritos\000Los Altos\000Bajo Saltos\000Bajo Barrientos\000Cotos\000Coyoles Motos"
    "\000Monte Olivos\000Pozos\000Tres R\303\255os\000San Francisco de Dos R\303\255os\000Bur\303\255os"
    "\000Colonia Bola\303\261os\000Casta\303\261os\000Joaquine\303\261os\000Reto\303\261os\000Bajo Seever"
    "s\000Taus\000Villa Emaus\000Coto Brus\000Jardines de Tib\303\241s\000Chuc\303\241s\000San Nicol\303"
    "\241s\000Santo Tom\303\241s\000Po\303\241s\000Nueva Ujarr\303\241s\000Holand\303\251s\000Avil\303"
    "\251s\000Santa In\303\251s\000San Andr\303\251s\000Cipr\303\251s\000Villa Cort\303\251s\000Punta Cor"
    "t\303\251s\000Llanos de Cort\303\251s\000Cerro Cort\303\251s\000Puerto Cort\303\251s\000Malpa\303"
    "\255s\000Ib\303\255s\000Sol\303\255s\000Ip\303\255s\000Colonia Par\303\255s\000Pirr\303\255s\000As"
    "\303\255s\000Itiqu\303\255s\000Dulce Nombre de Jes\303\272s\000Coraz\303\263n de Jes\303\272s\000Pue"
    "rto Jes\303\272s\000R\303\255o Jes\303\272s\000Curridabat\000Sagamat\000Monserrat\000Montserrat\000L"
    "lobet\000Bajo Canet\000Claret\000Nazaret\000Carit\000Roosevelt\000Kent\000Zent\000Cot\000Quircot\000"
    "Margot\000Maalot\000Coopelot\000Cansot\000Penshurt\000Sukut\000Ourut\000Kivut\000Don Chu\000Ca\303"
    "\261o Chu\000Mountain Cow\000Sand Box\000Berlay\000Irigaray\000Juray\000Magsaysay\000Guay\000Kennedy"
    "\000R\303\255o Ley\000Bajo El Rey\000San Esteban Rey\000Cristo Rey\000Biolley\000Puerto Soley\000Bev"
    "erley\000Bajo Mamey\000Copey\000Monterrey\000Cabeza de Buey\000Soky\000July\000Jabuy\000Duruy\000Sur"
    "uy\000La Paz\000Jardines de la Paz\000Juan D\303\255az\000Bajos de Fern\303\241ndez\000Altos Fern"
    "\303\241ndez\000Hern\303\241ndez\000Fila M\303\251ndez\000Alto M\303\251ndez\000Jos\303\251 Jes\303"
    "\272s M\303\251ndez\000Bajo Berm\303\272dez\000S\303\241nchez\000Finca Zona Diez\000Calle Diez\000Go"
    "nz\303\241lez\000Piedra P\303\263mez\000Puerto Jimenez\000Carlos Mar\303\255a Jim\303\251nez\000Boca"
    " R\303\255o Jim\303\251nez\000Pilar Jim\303\251nez\000Manuel Tucker Mart\303\255nez\000Ultrapez\000A"
    "lto L\303\263pez\000S\303\241urez\000Su\303\241rez\000Bajo P\303\251rez\000Calle Ram\303\255rez\000B"
    "ajo Ram\303\255rez\000Gregorio Jos\303\251 Ram\303\255rez\000Portuguez\000Corina Rodr\303\255guez"
    "\000Rinc\303\263n Rodr\303\255guez\000Bajo Rodr\303\255guez\000Aranjuez\000V\303\241squez\000Vel\303"
    "\241squez\000Puerto Gonz\303\241lez V\303\255quez\000Cultivez\000La Ch\303\241vez\000Flor de Liz\000"
    "Chiz\000Calder\303\263n Mu\303\261oz\000Lutz\000Bella Luz\000Llano La Cruz\000Alto La Cruz\000Rinc"
    "\303\263n de la Cruz\000Santa Cruz\000Alto Cruz\000Maruz\000Veracruz\000Aranju\303\251z\000Bajo Ma"
    "\303\255z\000Valle Fel\303\255z\000Garcimu\303\261\303\263z\000Veracr\303\272z\000Purab\303\241\000P"
    "aso Verb\303\241\000Canad\303\241\000China Kich\303\241\000Diri\303\241\000Salam\303\241\000Panam"
    "\303\241\000Man\303\241\000Par\303\241\000Arrabar\303\241\000Guaycar\303\241\000Patarr\303\241\000Ti"
    "rr\303\241\000Macu\303\241\000San Juan de Chicu\303\241\000Santa F\303\251\000Curuband\303\251\000Ce"
    "rro Caf\303\251\000Yomal\303\251\000San Bartolom\303\251\000Curr\303\251\000Colonia San Jos\303\251"
    "\000Vista de San Jos\303\251\000Bajo San Jos\303\251\000Josu\303\251\000Sina\303\255\000Tirib\303"
    "\255\000Namb\303\255\000Rub\303\255\000Corobic\303\255\000Pococ\303\255\000Namald\303\255\000Zapand"
    "\303\255\000Cach\303\255\000Copalch\303\255\000Sarch\303\255\000Guaym\303\255\000Duacar\303\255\000T"
    "icar\303\255\000Guarar\303\255\000Bribr\303\255\000Quever\303\255\000Dindir\303\255\000Sanchir\303"
    "\255\000Camaquir\303\255\000Tacacor\303\255\000Lomas de Cocor\303\255\000Tocor\303\255\000Lomas de A"
    "serr\303\255\000Plan de Birr\303\255\000Cirr\303\255\000Sur\303\255\000Quitirris\303\255\000Oros\303"
    "\255\000Tapant\303\255\000Laqu\303\255\000Zaiqu\303\255\000Sarapiqu\303\255\000Zurqu\303\255\000Pasq"
    "u\303\255\000Purisqu\303\255\000Cubujuqu\303\255\000Korob\303\263\000Jac\303\263\000Jeric\303\263"
    "\000Chirc\303\263\000Calle Juc\303\263\000Chirrip\303\263\000Alto Por\303\263\000La Bamb\303\272\000"
    "Colonia Bamb\303\272\000Matamb\303\272\000Chambac\303\272\000Sibuj\303\272\000Man\303\272\000Itaip"
    "\303\272\000Bar\303\272\000Istar\303\272\000Per\303\272\000Cur\303\272\000Kur\303\272\000Tur\303\272"
    "\000Sorobat\303\272\000Cant\303\272\000Escaz\303\272\000San Rafael de Iraz\303\272\000Cabeceras de T"
    "arraz\303\272\000Bajos de Tarraz\303\272\000";

constexpr uint16_t kProvNombre[] = {
    49411, 10682, 27609, 9511, 20675, 42327, 25243,
};

// Provincia p tiene los cantones [kProvCantonIni[p-1], kProvCantonIni[p]).
constexpr uint8_t kProvCantonIni[] = {
    0, 20, 36, 44, 54, 65, 78, 84,
};

constexpr uint16_t kCantonNombre[] = {
    49411, 49987, 46287, 21729, 50029, 49676, 13828, 8783, 11670, 14931, 26895, 15890, 47749, 10265,
    7392, 44833, 15673, 48070, 24806, 37284, 10682, 25227, 9347, 27518, 42373, 29215, 44988, 47790,
    12453, 46941, 31368, 49543, 10560, 44352, 37446, 38807, 27609, 37454, 25006, 48707, 7295, 26948,
    30932, 26349, 9511, 16289, 27701, 12919, 23142, 31311, 24513, 45288, 29419, 49755, 10059, 16464,
    49108, 43600, 29885, 43505, 44920, 24392, 19086, 49068, 9195, 42327, 16812, 45157, 31057, 14377,
    47148, 37715, 47727, 15326, 45248, 37496, 18010, 48678, 25243, 49500, 45382, 7829, 12423, 30245,
};

constexpr uint16_t kCantonDistIni[] = {
    0, 11, 14, 27, 36, 39, 46, 53, 60, 66, 71, 76, 81, 86,
    89, 93, 98, 101, 105, 117, 123, 137, 151, 158, 162, 170, 178, 185,
    190, 195, 208, 215, 220, 228, 232, 236, 239, 250, 256, 264, 268, 280,
    283, 288, 292, 297, 304, 312, 318, 323, 327, 330, 333, 335, 340, 345,
    352, 361, 365, 369, 374, 378, 386, 392, 396, 401, 417, 423, 432, 435,
    441, 444, 448, 454, 455, 459, 462, 463, 464, 468, 475, 482, 486, 489,
    494,
};

constexpr uint16_t kDistNombre[] = {
    24052, 17557, 22739, 22442, 19782, 47613, 8068, 8706, 43329, 29951, 24305, 49987, 28308, 23142,
    46287, 23337, 46505, 7035, 28308, 44339, 49297, 21590, 28418, 42114, 28737, 41588, 27153, 27560,
    40809, 42541, 38433, 23142, 15241, 47417, 28308, 45205, 46208, 39188, 46941, 49676, 7446, 26399,
    23269, 16093, 48418, 46892, 25193, 25742, 9495, 42738, 45114, 45936, 49712, 18636, 26553, 46180,
    30674, 47958, 27281, 22595, 11670, 22602, 47596, 8068, 43688, 23452, 14931, 37543, 28308, 24977,
    18590, 31311, 23142, 48002, 29744, 22007, 27934, 23467, 21914, 22016, 41978, 23989, 42468, 19484,
    6702, 11511, 19386, 30254, 17425, 31285, 10987, 43741, 23142, 29419, 31285, 14666, 45982, 12982,
    16952, 24557, 48412, 48070, 10895, 48618, 45420, 22478, 22467, 45295, 43362, 31285, 44997, 21063,
    25080, 49932, 38986, 30237, 17538, 29419, 47841, 38048, 31311, 49108, 28308, 10682, 49411, 23071,
    28308, 11496, 31311, 10987, 23142, 27342, 46287, 44881, 40196, 15160, 49755, 25227, 27560, 23989,
    20377, 40796, 23142, 31311, 44222, 31121, 28252, 24977, 22859, 41054, 39188, 9347, 31311, 49411,
    20991, 44844, 13167, 40016, 27518, 19627, 16984, 40256, 42373, 48018, 43741, 31311, 24977, 49411,
    9616, 21545, 29215, 23337, 49411, 40904, 30254, 23989, 28418, 47462, 44988, 16752, 45157, 27560,
    9979, 42092, 10458, 31285, 23989, 23142, 46872, 8691, 12453, 19232, 10353, 39754, 6777, 8330,
    9477, 15846, 41110, 9339, 22698, 12666, 13723, 13421, 26876, 45956, 48418, 23741, 31368, 12659,
    26708, 18636, 13793, 19782, 42878, 20490, 40892, 29851, 31285, 48893, 10560, 42611, 19934, 16016,
    41546, 47630, 22109, 19272, 44352, 31721, 31172, 18237, 23142, 15846, 19662, 13808, 38807, 14837,
    23197, 22783, 22765, 24052, 47764, 26567, 10997, 29733, 7684, 18781, 17891, 10921, 37454, 27560,
    21369, 49526, 16863, 38214, 47602, 27629, 23989, 23142, 24977, 18781, 25227, 23916, 43561, 20964,
    21063, 10131, 7295, 16548, 15522, 49108, 15341, 44717, 45991, 17400, 14520, 45996, 23174, 49845,
    43368, 45543, 43716, 23142, 48169, 27060, 45446, 14520, 39616, 31311, 21362, 15960, 9511, 43741,
    26553, 12725, 7672, 16289, 31285, 29419, 20991, 16873, 17143, 41793, 27701, 19386, 23337, 37534,
    47777, 14520, 45409, 49271, 12919, 31285, 23989, 48018, 27701, 49201, 23142, 37543, 27560, 44222,
    24977, 31311, 49411, 24977, 37374, 28308, 13236, 24130, 24704, 45533, 19492, 29419, 10976, 29082,
    24025, 42995, 39913, 17294, 10059, 43619, 9117, 30156, 49350, 16464, 25053, 28308, 8661, 12974,
    12995, 15262, 49108, 25439, 23425, 19224, 11979, 23482, 49242, 41824, 27223, 43600, 12666, 19694,
    29280, 9562, 13793, 22258, 24513, 43505, 13793, 23337, 31415, 23088, 43294, 14142, 23989, 27016,
    24392, 17722, 14004, 14520, 30456, 42357, 22246, 42660, 12596, 14837, 22859, 29419, 40181, 26663,
    49068, 9692, 15157, 11989, 9195, 30278, 29878, 40997, 49891, 42327, 16352, 44490, 38554, 13606,
    29789, 22201, 7859, 65535, 26323, 30464, 15194, 13778, 26112, 18404, 9302, 38515, 17846, 12522,
    23142, 30254, 13264, 45157, 24268, 17904, 8083, 41904, 42438, 12143, 48371, 10504, 39815, 25006,
    31311, 47921, 39855, 18621, 12013, 41038, 18379, 47148, 18931, 37757, 37715, 65535, 49287, 25515,
    37487, 37781, 12113, 37654, 40095, 24192, 15326, 40301, 15743, 42556, 23304, 49811, 44419, 41999,
    18010, 48678, 25243, 10769, 26184, 11469, 44363, 48707, 14843, 11941, 21276, 27016, 9940, 45382,
    38100, 8494, 9808, 31752, 17035, 25554, 21399, 11286, 15390, 19002, 12423, 24455, 21213, 30245,
    43741, 13978, 48721, 49559,
};

constexpr uint16_t kDistBarrioIni[] = {
    0, 8, 17, 35, 55, 73, 91, 112, 129, 156, 173, 200, 209, 235,
    263, 296, 323, 334, 346, 357, 362, 373, 378, 387, 394, 410, 422, 425,
    445, 468, 475, 482, 487, 494, 498, 507, 533, 551, 583, 592, 610, 614,
    625, 635, 642, 645, 656, 676, 681, 690, 695, 701, 703, 707, 728, 731,
    741, 758, 772, 779, 793, 803, 811, 823, 830, 837, 840, 846, 854, 860,
    870, 878, 894, 901, 913, 925, 942, 961, 976, 996, 1010, 1029, 1045, 1053,
    1066, 1069, 1076, 1107, 1112, 1123, 1145, 1157, 1164, 1176, 1180, 1186, 1193, 1199,
    1225, 1241, 1245, 1269, 1290, 1297, 1301, 1306, 1393, 1409, 1441, 1473, 1492, 1514,
    1541, 1561, 1589, 1605, 1625, 1635, 1646, 1653, 1662, 1667, 1672, 1675, 1707, 1728,
    1735, 1748, 1761, 1783, 1796, 1804, 1816, 1829, 1846, 1854, 1867, 1878, 1885, 1901,
    1913, 1923, 1940, 1955, 1961, 1970, 1977, 1982, 1987, 1997, 2012, 2023, 2033, 2044,
    2053, 2063, 2075, 2087, 2094, 2106, 2115, 2119, 2124, 2133, 2142, 2147, 2154, 2161,
    2168, 2170, 2178, 2195, 2203, 2213, 2227, 2231, 2238, 2244, 2252, 2255, 2262, 2269,
    2271, 2273, 2278, 2280, 2288, 2295, 2306, 2313, 2316, 2334, 2338, 2342, 2354, 2366,
    2431, 2457, 2462, 2493, 2513, 2551, 2578, 2592, 2608, 2623, 2671, 2692, 2745, 2748,
    2750, 2751, 2755, 2757, 2761, 2764, 2772, 2781, 2783, 2789, 2792, 2814, 2832, 2863,
    2881, 2893, 2901, 2917, 2928, 2973, 2983, 3006, 3023, 3058, 3068, 3076, 3090, 3105,
    3115, 3124, 3143, 3153, 3166, 3189, 3202, 3208, 3226, 3235, 3242, 3250, 3260, 3280,
    3297, 3315, 3328, 3332, 3333, 3336, 3344, 3351, 3359, 3369, 3372, 3381, 3385, 3398,
    3409, 3429, 3430, 3464, 3489, 3491, 3509, 3531, 3542, 3558, 3570, 3578, 3587, 3595,
    3615, 3629, 3638, 3643, 3660, 3665, 3671, 3677, 3686, 3699, 3738, 3745, 3749, 3767,
    3777, 3795, 3810, 3817, 3820, 3831, 3835, 3840, 3847, 3866, 3867, 3868, 3873, 3880,
    3882, 3886, 3893, 3897, 3900, 3902, 3905, 3909, 3919, 3926, 3933, 3937, 3943, 3947,
    3961, 3974, 3981, 3989, 3994, 4002, 4008, 4014, 4021, 4029, 4032, 4041, 4061, 4066,
    4109, 4140, 4181, 4190, 4201, 4245, 4262, 4269, 4276, 4282, 4338, 4365, 4405, 4422,
    4447, 4467, 4485, 4531, 4536, 4581, 4590, 4600, 4628, 4642, 4660, 4676, 4698, 4715,
    4730, 4733, 4748, 4755, 4781, 4801, 4841, 4854, 4862, 4866, 4876, 4911, 4926, 4936,
    4949, 4961, 4967, 4972, 4981, 4986, 4995, 5000, 5007, 5013, 5023, 5037, 5046, 5054,
    5085, 5121, 5143, 5158, 5163, 5176, 5189, 5199, 5204, 5209, 5219, 5229, 5252, 5305,
    5350, 5356, 5366, 5373, 5373, 5374, 5416, 5431, 5440, 5448, 5452, 5460, 5468, 5472,
    5478, 5484, 5492, 5516, 5557, 5574, 5623, 5644, 5660, 5681, 5700, 5713, 5725, 5741,
    5751, 5758, 5791, 5824, 5860, 5879, 5904, 5918, 5953, 5976, 5991, 6051, 6051, 6071,
    6104, 6125, 6159, 6179, 6202, 6214, 6229, 6286, 6341, 6346, 6374, 6403, 6418, 6439,
    6440, 6441, 6442, 6481, 6536, 6553, 6575, 6599, 6616, 6651, 6680, 6713, 6722, 6730,
    6765, 6793, 6811, 6820, 6841, 6847, 6865, 6884, 6902, 6927, 6964, 6986, 7010, 7041,
    7058, 7063, 7066, 7080, 7089,
};

constexpr uint16_t kBarrioNombre[] = {
    4314, 25213, 48939, 1130, 24052, 18564, 19341, 16477, 4314, 25016, 48128, 11312, 45332, 7635, 26104, 7430,
    16352, 4314, 44893, 44222, 40016, 48143, 2734, 48025, 48360, 7387, 1680, 17557, 2819, 47057, 17502, 26623,
    26553, 16873, 46787, 4314, 15829, 1130, 48693, 1680, 47053, 1846, 24112, 48975, 23527, 49068, 31856, 24320,
    40332, 41271, 2819, 3095, 17408, 13339, 9904, 33810, 8275, 49030, 38167, 7367, 10100, 24557, 42908, 46413,
    18919, 43512, 24409, 42166, 2915, 23646, 47802, 14574, 40835, 36815, 3997, 21008, 43537, 41579, 10013, 31108,
    11518, 30603, 8944, 24104, 26544, 48188, 48561, 7513, 43640, 47408, 49766, 32192, 8275, 42182, 44410, 45392,
    16732, 48025, 22933, 37243, 15765, 10321, 28347, 9758, 23392, 48089, 12379, 21749, 29551, 40076, 30908, 11166,
    4314, 41014, 937, 18022, 2734, 30782, 8561, 14474, 18449, 8646, 47045, 16524, 12637, 3436, 11589, 11699,
    40010, 36145, 8891, 41654, 24957, 49585, 20087, 40461, 9354, 11630, 9826, 17529, 39333, 23631, 12217, 45861,
    14131, 38913, 20777, 17862, 3436, 8742, 31285, 12919, 12248, 49340, 30191, 16665, 35201, 937, 30611, 6138,
    6254, 6342, 6419, 6477, 6539, 6563, 6617, 18741, 9719, 49469, 27946, 28258, 9256, 34478, 10542, 25631,
    23541, 24052, 22007, 23895, 21266, 28577, 16377, 24305, 28334, 48315, 27528, 21489, 48764, 46255, 37723, 23873,
    21257, 27848, 45563, 30636, 26553, 14520, 28408, 49970, 37228, 23066, 2543, 44438, 23688, 13414, 40923, 29620,
    3959, 35024, 11841, 31415, 2255, 2543, 39409, 7981, 22578, 29915, 47375, 46125, 7763, 11886, 41686, 11185,
    15908, 31518, 49946, 3959, 41978, 46892, 45836, 14433, 46814, 30003, 31032, 34216, 47112, 10573, 47107, 42219,
    2255, 12430, 6780, 37259, 24590, 13490, 44299, 24838, 21767, 29401, 17592, 40280, 44392, 30307, 7988, 39885,
    13650, 8330, 1562, 15698, 11958, 18912, 18872, 36633, 13784, 15829, 41921, 49621, 11099, 46345, 7551, 40577,
    45237, 17704, 45045, 46300, 15309, 12669, 24557, 38779, 21246, 10025, 31148, 17941, 47057, 47688, 26445, 12907,
    48344, 30254, 49411, 20991, 47383, 47086, 9339, 17974, 34287, 44222, 21356, 43050, 23724, 38291, 15818, 26415,
    30508, 40162, 47589, 41382, 21719, 8210, 22100, 10987, 24671, 45836, 18145, 18794, 39087, 49817, 30716, 16330,
    39675, 3910, 46719, 36653, 43009, 25618, 10480, 49924, 42518, 15678, 43697, 18182, 39266, 44196, 32079, 30834,
    45926, 49869, 48278, 10006, 3255, 43879, 41686, 12034, 17079, 48561, 35024, 41538, 40223, 9415, 8090, 45016,
    14782, 38913, 42145, 41323, 39739, 36321, 50038, 19365, 1368, 14774, 37118, 19129, 28387, 26648, 37446, 31245,
    24502, 17675, 42849, 8661, 3910, 20151, 2119, 1030, 40495, 14142, 35046, 18980, 45130, 11458, 16436, 1234,
    38048, 8661, 2033, 11518, 14906, 26972, 41123, 13561, 16608, 39188, 35061, 44222, 45268, 12677, 48195, 9843,
    44681, 27480, 25837, 28426, 9692, 3407, 37387, 41280, 9452, 16337, 35910, 13446, 44308, 26470, 19351, 41287,
    12669, 15140, 40181, 16322, 15530, 16211, 34862, 42827, 18022, 34938, 44222, 45157, 16965, 22622, 7202, 38913,
    31311, 10867, 42796, 28900, 7248, 48143, 2679, 49699, 28850, 47596, 26553, 24671, 19782, 35780, 11559, 42252,
    16088, 15467, 8338, 11726, 12596, 11876, 46913, 22503, 17891, 20392, 11754, 8661, 23385, 27743, 44069, 15698,
    44904, 24619, 13951, 22859, 36053, 42536, 46442, 23043, 37446, 43688, 23989, 35613, 25644, 13833, 28599, 49860,
    38913, 29927, 7035, 22962, 9630, 16557, 28737, 33137, 10880, 44588, 39658, 22380, 7651, 42263, 36746, 13031,
    15598, 8210, 35024, 13531, 13517, 20686, 2679, 31581, 29095, 22602, 20719, 48360, 24989, 17337, 44222, 1643,
    22246, 45200, 22210, 48332, 8926, 41337, 19124, 62, 5335, 4848, 10100, 22218, 37834, 22681, 46172, 3209,
    23337, 19386, 14520, 39517, 25447, 36615, 8852, 8944, 43461, 9692, 27484, 28571, 14118, 48117, 23984, 48122,
    2495, 18636, 13201, 30636, 30315, 1300, 31285, 35665, 22227, 22914, 22880, 10933, 8656, 45715, 22867, 12876,
    24977, 19214, 41546, 16598, 42092, 16327, 43335, 17105, 39815, 44429, 29666, 11542, 39100, 27484, 1300, 26799,
    27440, 26553, 24671, 9692, 15698, 14520, 22859, 36714, 22533, 4879, 31159, 49440, 7505, 139, 30254, 37543,
    37165, 6713, 31884, 3650, 48025, 23688, 43750, 49667, 43829, 13985, 42849, 47790, 14837, 48795, 43465, 8473,
    37446, 40280, 32121, 8896, 10381, 18944, 34721, 28444, 1827, 22807, 29250, 46413, 14812, 27267, 15974, 16922,
    8068, 34254, 38913, 1234, 22351, 30567, 17877, 22602, 42040, 2033, 16211, 33317, 15874, 19154, 22957, 42513,
    3509, 2609, 36995, 1907, 48876, 36695, 3650, 15341, 31911, 17803, 29721, 3255, 41999, 26145, 41205, 24757,
    34645, 13089, 23452, 42719, 26922, 24247, 23446, 23130, 26623, 19386, 13373, 11343, 22435, 22824, 24859, 25265,
    8661, 27484, 17454, 45094, 34706, 42599, 26979, 45140, 31662, 32355, 19360, 11486, 16529, 43505, 39666, 26984,
    41038, 38474, 36084, 18443, 15584, 11542, 17722, 36433, 23228, 47756, 27512, 17891, 5984, 36535, 37482, 37180,
    23989, 24671, 8661, 33665, 44410, 39254, 3339, 11518, 9438, 24557, 9758, 11253, 15207, 16295, 43512, 8773,
    48735, 18331, 27460, 9692, 45836, 38858, 25009, 21406, 34759, 12711, 25290, 36594, 47853, 38546, 11072, 39836,
    47057, 28308, 23269, 47777, 28252, 35301, 41069, 24052, 22980, 49071, 2097, 1315, 41663, 22076, 48478, 43829,
    42670, 17380, 6762, 15504, 18118, 18467, 36910, 44222, 3339, 15734, 49803, 13825, 15302, 22906, 30739, 8773,
    27910, 1504, 22170, 19550, 34879, 40280, 29733, 46802, 11424, 23337, 39517, 34127, 21479, 45373, 47185, 46973,
    3339, 49013, 49958, 17371, 15150, 23732, 43888, 29446, 42986, 32690, 41298, 43537, 7698, 3596, 27190, 2194,
    23142, 45140, 41904, 34141, 2194, 29677, 31346, 21620, 11045, 25169, 26059, 36784, 42232, 24977, 43355, 12847,
    44518, 42834, 3596, 14040, 21986, 1562, 23617, 31099, 7340, 23688, 17778, 12157, 2194, 8063, 36255, 17135,
    10885, 43171, 23600, 46200, 27547, 34333, 23290, 48412, 33085, 15829, 8967, 11089, 9145, 21863, 35511, 26783,
    8794, 38425, 20899, 47375, 27119, 14921, 35024, 40977, 10611, 30508, 27639, 42624, 34579, 47230, 19535, 26094,
    38414, 23411, 37345, 3407, 49421, 21777, 33633, 14111, 48886, 46099, 23400, 15522, 11963, 30590, 35359, 46984,
    44760, 41637, 14468, 47198, 48025, 30947, 3839, 40997, 41598, 43741, 23873, 26553, 24671, 15787, 34216, 30105,
    8944, 15085, 43594, 15960, 47715, 36923, 13325, 24052, 41844, 49455, 47026, 22926, 17619, 49165, 1268, 44997,
    2935, 28308, 9683, 3839, 45614, 50009, 44556, 40256, 29744, 15698, 18263, 8635, 16260, 31285, 47821, 28168,
    6771, 39168, 49462, 12731, 22007, 26337, 11424, 48089, 46904, 2935, 10214, 42357, 25071, 15016, 34992, 7954,
    22588, 13985, 8994, 47596, 5240, 45982, 22043, 8473, 918, 13350, 13937, 44222, 800, 16598, 46860, 25110,
    39147, 34347, 14172, 16896, 41643, 48603, 39039, 42306, 42796, 39754, 8220, 49068, 3610, 7920, 27089, 19782,
    34015, 29419, 898, 41332, 26430, 40016, 41239, 22524, 48025, 29709, 800, 10718, 39767, 3610, 12154, 14819,
    18207, 16371, 29419, 11263, 34032, 43672, 1827, 10310, 20504, 13107, 41588, 15818, 38048, 14401, 22026, 31978,
    39619, 45220, 35940, 25465, 11548, 48811, 22962, 25601, 11369, 27016, 19138, 42176, 25470, 30289, 42263, 30254,
    17408, 23364, 47505, 8068, 30697, 34379, 41538, 8445, 41654, 45511, 8494, 21015, 14060, 15818, 42928, 30254,
    45836, 18461, 44382, 7839, 9874, 36031, 45499, 48412, 39126, 41893, 39891, 30180, 14433, 33770, 30166, 9600,
    9390, 40086, 29203, 47737, 17416, 21836, 42710, 7588, 7829, 23235, 32045, 11329, 37496, 32676, 26940, 14508,
    42391, 41382, 48340, 24691, 33751, 13214, 30472, 41014, 11419, 41852, 10746, 16032, 24052, 46597, 20459, 45682,
    3339, 40316, 9477, 10006, 22776, 11424, 10253, 14381, 13394, 44196, 22459, 25537, 41767, 18237, 24671, 23142,
    1504, 47777, 14657, 35232, 1268, 44997, 22130, 19053, 4584, 17474, 45446, 9943, 22545, 31500, 22654, 30691,
    46685, 45666, 11177, 35342, 13089, 41393, 23674, 22435, 4179, 1846, 45575, 10461, 10304, 43829, 48418, 21349,
    19463, 38626, 20543, 48149, 2915, 15698, 14657, 16398, 45473, 32600, 8445, 13624, 47198, 3389, 14950, 11765,
    10806, 11253, 27484, 44085, 30841, 9835, 40150, 11788, 4179, 49551, 27245, 12321, 34216, 8454, 8462, 9922,
    2097, 3389, 1315, 12840, 9585, 3857, 49129, 46892, 35125, 49855, 25439, 45212, 35342, 25278, 10840, 25243,
    39855, 12287, 5478, 12654, 31881, 29800, 45103, 21581, 31674, 36548, 22698, 6938, 23791, 26553, 23142, 22962,
    23372, 1643, 37900, 11271, 12982, 11481, 41546, 40488, 16598, 13700, 24333, 12822, 41686, 30620, 39836, 22998,
    12622, 12471, 31710, 25933, 26553, 23269, 31311, 30254, 24619, 33440, 13272, 22369, 30508, 43594, 23142, 10815,
    22435, 10006, 29215, 2050, 45720, 24704, 41156, 9594, 40340, 34544, 50016, 1872, 41965, 36982, 17228, 39155,
    28180, 39823, 2772, 11481, 10959, 8494, 8883, 29620, 16249, 1907, 2965, 37422, 9426, 2050, 40265, 26184,
    3763, 46941, 27460, 17428, 43259, 36953, 3997, 26083, 45446, 17388, 46300, 46125, 10353, 39603, 24784, 12659,
    16836, 7997, 13985, 45583, 13364, 23572, 26934, 49411, 9692, 25830, 32563, 10086, 38784, 45439, 20018, 40387,
    1697, 37025, 1182, 26376, 45008, 36486, 12298, 27954, 14632, 38883, 34095, 38839, 37410, 8083, 24181, 28476,
    16231, 48360, 39212, 8875, 26731, 17127, 28110, 24142, 48654, 22739, 25530, 8944, 41010, 49642, 18196, 24478,
    44717, 37422, 37664, 26934, 31473, 9719, 47841, 45982, 40562, 19386, 9692, 49462, 8502, 15622, 26478, 18145,
    25770, 40992, 22422, 23998, 40937, 14367, 25759, 43406, 13912, 44153, 11726, 15053, 25753, 16541, 26972, 43411,
    18636, 10006, 44681, 31588, 40719, 46121, 13985, 44392, 19292, 15974, 18699, 38100, 11542, 14884, 38794, 958,
    43250, 41205, 18398, 28418, 24691, 18237, 39803, 39188, 20185, 49340, 15698, 1962, 28631, 27089, 43981, 45991,
    16211, 28951, 12796, 24052, 46462, 14566, 8596, 6832, 45348, 14184, 41054, 4296, 41767, 45982, 49108, 11989,
    17428, 44988, 42885, 44222, 14111, 44356, 28531, 45295, 28551, 47063, 17963, 43829, 42928, 45905, 26553, 15217,
    9249, 9573, 3629, 39994, 2696, 48025, 39977, 19652, 15960, 18357, 21712, 39318, 958, 15665, 45720, 13239,
    1962, 36158, 15753, 43829, 10516, 13784, 39762, 44222, 19307, 15846, 24222, 23707, 12847, 11654, 2716, 18636,
    14261, 49866, 43852, 48418, 22722, 15481, 8324, 43376, 38913, 26184, 28308, 27460, 49411, 20249, 1149, 49306,
    22859, 35342, 10480, 298, 25333, 2495, 11518, 12669, 10006, 12659, 8911, 30254, 23989, 37602, 23142, 11670,
    27701, 40196, 25009, 22859, 23142, 3629, 43123, 16761, 10288, 43128, 45157, 2696, 48360, 14139, 43829, 22681,
    37615, 44626, 46725, 37923, 29419, 31970, 2400, 12437, 40310, 39517, 33928, 22831, 10739, 4687, 19430, 42380,
    39015, 15829, 29968, 41546, 46287, 37342, 18693, 18636, 43661, 42849, 42385, 37366, 28308, 46208, 24671, 23337,
    17610, 2400, 17428, 49135, 19782, 34631, 2495, 26817, 10100, 42897, 41420, 43741, 30180, 39696, 43594, 37454,
    39725, 38913, 4296, 44069, 26553, 27934, 38110, 16952, 14433, 29765, 13098, 11400, 46692, 22850, 49932, 37671,
    25644, 44661, 7805, 6780, 44133, 41934, 8494, 5088, 30456, 9758, 47596, 12224, 46208, 40288, 11922, 38858,
    4614, 38150, 3673, 40942, 39495, 14520, 11948, 29319, 13922, 30482, 10100, 21630, 30508, 37454, 1049, 10496,
    38986, 28308, 15698, 28709, 26518, 40562, 13896, 44222, 24611, 13297, 2716, 12669, 10234, 19096, 13116, 13729,
    47016, 11542, 38237, 1049, 20296, 45836, 47777, 1149, 9452, 28308, 29733, 49229, 11660, 28542, 46941, 23269,
    20991, 9692, 14483, 35125, 28110, 12953, 24025, 44276, 9719, 22328, 44185, 31899, 31057, 28418, 36869, 1643,
    12699, 25354, 17891, 1907, 43926, 7035, 13912, 3509, 24977, 26553, 45982, 28737, 11922, 1661, 35359, 23066,
    14319, 1661, 17428, 37100, 2495, 1030, 24671, 12684, 35024, 1643, 15746, 32547, 2074, 16931, 39109, 23052,
    2560, 22690, 42556, 24052, 28476, 24977, 41874, 48025, 48360, 48852, 18636, 44681, 22739, 30508, 48110, 47078,
    4031, 31733, 4161, 45309, 9426, 31761, 45982, 9537, 11706, 1778, 14602, 37134, 17541, 7578, 7774, 45288,
    44556, 19687, 16998, 11089, 1444, 1250, 2575, 38913, 23142, 14837, 13723, 19053, 17428, 40538, 23767, 45288,
    34185, 43324, 49869, 42468, 9552, 41502, 23971, 39625, 2074, 18171, 8420, 4031, 4161, 1404, 39619, 41424,
    1778, 41874, 18398, 48618, 32660, 39647, 25644, 8420, 26310, 28591, 45726, 30210, 38330, 13498, 18154, 42283,
    43259, 35359, 39731, 6780, 24671, 41910, 47640, 22300, 22160, 18781, 3116, 47993, 12659, 17051, 1250, 17159,
    41904, 46860, 30254, 23142, 49632, 2304, 6797, 32600, 39416, 44222, 12914, 31911, 10511, 3116, 44497, 41686,
    38201, 45982, 23142, 3530, 34216, 48471, 11405, 8420, 15974, 45193, 46860, 41468, 34901, 44222, 3743, 8750,
    20947, 9968, 3748, 25742, 4161, 1404, 46154, 48985, 36633, 3743, 15829, 2560, 38531, 3748, 39182, 25103,
    38194, 24757, 3783, 45429, 42647, 36398, 14827, 10461, 13476, 24671, 14837, 17244, 44852, 23348, 9979, 12057,
    10885, 4805, 29175, 43298, 44939, 27223, 35730, 25644, 44327, 48654, 41205, 25668, 2304, 3530, 33106, 515,
    8300, 23006, 44602, 23757, 47001, 1444, 46925, 12157, 2575, 23210, 29959, 23337, 42957, 26246, 48025, 15073,
    12557, 28308, 37454, 10531, 47802, 3228, 34659, 13389, 13280, 17058, 1300, 49411, 2417, 34938, 41637, 48099,
    27555, 41806, 14367, 12677, 31489, 9415, 31785, 18564, 19096, 24838, 44528, 8105, 48058, 34379, 11716, 46698,
    24513, 45446, 46678, 25315, 38913, 20975, 39619, 10851, 3714, 33850, 24385, 16392, 4065, 40016, 47160, 16598,
    48471, 42213, 20881, 35760, 7892, 7859, 21847, 24052, 23782, 10006, 17363, 11542, 46860, 41965, 40292, 26623,
    26553, 23337, 22258, 31970, 34216, 43465, 25009, 30525, 24611, 13325, 17068, 44539, 21141, 26724, 29515, 13858,
    26716, 24704, 14053, 35359, 37345, 48838, 48517, 10006, 10662, 40822, 44552, 13288, 20407, 14508, 3714, 28038,
    23862, 9006, 31285, 40040, 1300, 2417, 4065, 14731, 18145, 35011, 41438, 44949, 18781, 24883, 34579, 10700,
    7135, 22568, 48816, 28308, 30080, 46692, 11240, 12057, 22292, 37846, 45674, 10134, 2527, 31311, 26043, 14330,
    21008, 38186, 31755, 3043, 38940, 17437, 15999, 49068, 23741, 46941, 23142, 44260, 23862, 7362, 48923, 42968,
    17257, 14083, 45658, 40122, 46321, 41100, 18237, 32324, 12596, 17068, 25193, 48707, 47057, 41626, 28308, 19386,
    12263, 35359, 13650, 31121, 19724, 29155, 10174, 38662, 44681, 25432, 25103, 23022, 37134, 12028, 31272, 3545,
    2383, 11573, 48893, 20422, 40845, 33911, 19750, 13456, 42054, 30866, 7087, 23014, 22300, 13589, 23337, 36383,
    38626, 41430, 17320, 41337, 3545, 45288, 2383, 41904, 15598, 40181, 45473, 32884, 49860, 11263, 15513, 12443,
    47218, 42213, 11660, 27136, 47240, 41776, 44085, 44222, 25080, 30147, 29943, 23989, 45982, 14848, 34921, 10067,
    13325, 31235, 30774, 46287, 18781, 38291, 26392, 22451, 41185, 46505, 33794, 8300, 17529, 8661, 25243, 20102,
    9735, 30424, 19782, 33460, 37496, 2050, 15645, 35744, 13784, 28915, 8719, 2050, 36017, 44222, 8809, 25379,
    28357, 31214, 45969, 39033, 43484, 36939, 29687, 9105, 19386, 14792, 19535, 23482, 30115, 1927, 36289, 25080,
    25704, 29610, 14782, 35359, 29267, 25682, 24478, 1927, 43329, 24757, 34445, 17929, 14508, 15167, 44455, 39570,
    47141, 40878, 11476, 48783, 16093, 20474, 44752, 11036, 32404, 48904, 34000, 29894, 46991, 6187, 42563, 46413,
    41205, 13483, 35110, 41899, 24513, 14244, 41193, 13985, 37312, 38913, 38606, 4939, 23142, 9979, 1333, 3650,
    47853, 25624, 18781, 34287, 44511, 47697, 46692, 42195, 13553, 26553, 20799, 29776, 7859, 6892, 8196, 46287,
    11424, 7868, 31285, 4124, 27453, 35819, 14751, 18140, 9001, 17704, 13966, 43829, 38048, 40745, 14984, 8661,
    24757, 4124, 19782, 35232, 10585, 44196, 21612, 34379, 27569, 16169, 22218, 27325, 28308, 45473, 35043, 48816,
    47134, 30508, 15217, 18483, 36768, 29938, 42190, 1333, 3650, 24977, 30127, 20991, 36417, 17610, 19386, 33389,
    842, 8253, 24, 16741, 41517, 48948, 36449, 13063, 21817, 43465, 18461, 10134, 48823, 34938, 4051, 32462,
    4051, 35958, 18392, 842, 16496, 41776, 32517, 17541, 35342, 5797, 37305, 31911, 19214, 8555, 2984, 14053,
    40666, 1628, 48025, 23688, 19235, 20249, 3875, 34216, 14048, 8090, 16828, 19544, 38347, 14520, 2984, 47951,
    3875, 24268, 28610, 12543, 42483, 17454, 38454, 8585, 14095, 32271, 1628, 18964, 32769, 19129, 8445, 24052,
    23043, 20355, 48018, 32034, 48788, 45348, 29040, 23142, 19386, 43465, 45745, 41311, 16598, 18944, 33717, 20616,
    23805, 21536, 32495, 21924, 24977, 43280, 35700, 16965, 19903, 10885, 44363, 22351, 46413, 44626, 27655, 25587,
    30254, 14837, 32063, 47340, 22007, 16345, 30245, 27687, 6212, 8045, 47330, 23297, 15434, 19782, 32219, 49119,
    43737, 44222, 45926, 17681, 43824, 8351, 15829, 24052, 47322, 28476, 20955, 6110, 6226, 48025, 49490, 30852,
    30986, 43993, 18636, 49068, 10723, 15204, 7912, 19050, 46775, 20233, 49047, 25859, 43741, 18357, 28308, 27460,
    24671, 29419, 20991, 17610, 17986, 25009, 21825, 9362, 42245, 31534, 0, 20172, 40527, 25193, 18781, 44611,
    15818, 47026, 43238, 42195, 40181, 5289, 28801, 7154, 31311, 17143, 45982, 23142, 19386, 18807, 9639, 32338,
    10119, 5186, 16582, 47453, 22568, 42063, 4014, 30827, 46952, 29124, 30222, 39862, 14389, 27136, 23791, 23989,
    45982, 23142, 12962, 16952, 14837, 13461, 11505, 8921, 10347, 33263, 13081, 1462, 28308, 26623, 35874, 30866,
    21986, 38871, 21186, 26623, 26506, 14298, 13784, 5517, 22962, 44347, 391, 24012, 31721, 38167, 47907, 24977,
    44356, 15584, 41546, 44438, 10100, 25728, 15281, 5724, 37879, 49411, 37432, 9904, 40112, 32309, 24052, 48025,
    25750, 24557, 10097, 44760, 42878, 45157, 25742, 42962, 10806, 24542, 21186, 47496, 37454, 28997, 30636, 24671,
    25009, 34157, 21008, 7973, 21590, 44222, 30955, 8241, 46362, 13068, 40969, 48246, 5935, 42483, 25413, 4781,
    38546, 4925, 40316, 8944, 12230, 16093, 18299, 37748, 39855, 24898, 18858, 27136, 17722, 30960, 45982, 11989,
    18944, 13307, 41424, 49191, 2749, 25140, 24462, 32829, 13815, 46925, 47589, 22559, 37482, 31311, 49462, 23817,
    30231, 44222, 39402, 3043, 10006, 4564, 31588, 18062, 41382, 11542, 11405, 31311, 18237, 43837, 8053, 20975,
    42499, 17634, 32958, 44222, 42195, 47516, 16598, 37330, 4014, 9278, 27460, 31311, 49411, 23337, 31285, 23142,
    32925, 14330, 42120, 5019, 19979, 11573, 43829, 12363, 15035, 23142, 40562, 14520, 21581, 25009, 5159, 16939,
    34809, 15291, 16844, 39402, 25644, 41546, 16598, 38073, 15818, 1287, 25891, 31311, 1086, 16873, 13723, 22241,
    47230, 15829, 9835, 46933, 1044, 43181, 24052, 8113, 18646, 30147, 8984, 48025, 43032, 38175, 32002, 42728,
    25325, 14230, 45033, 29348, 24557, 40129, 14764, 24340, 37654, 10265, 30094, 38913, 27505, 26003, 18398, 28308,
    27197, 26553, 24704, 18237, 49411, 46208, 14837, 14433, 37487, 5435, 2890, 42283, 2749, 46615, 41257, 27701,
    46172, 29658, 27617, 47198, 49900, 41546, 25006, 21688, 40280, 39836, 48418, 8773, 7003, 37781, 47841, 28308,
    21590, 23337, 1086, 15698, 14520, 11518, 44289, 42080, 37432, 26112, 8767, 44222, 11971, 42691, 1044, 42878,
    15846, 45157, 23071, 6780, 37739, 27859, 42483, 38133, 31581, 10778, 10006, 24635, 19687, 11448, 14477, 18062,
    24478, 45689, 37454, 21799, 30289, 23741, 5893, 38913, 38502, 26828, 49484, 29134, 26623, 21553, 27629, 27460,
    31311, 24671, 23142, 9692, 16681, 16952, 2890, 41570, 44821, 35377, 42271, 15341, 32815, 17182, 34794, 33665,
    21310, 15829, 5916, 32974, 5545, 33810, 1462, 41674, 11989, 211, 44222, 16093, 33872, 44222, 27367, 38490,
    16125, 14477, 5210, 22267, 35800, 18258, 13429, 37272, 16149, 22088, 40056, 19188, 23337, 31832, 30295, 35342,
    37252, 9204, 25879, 9059, 41753, 23989, 49869, 10987, 32534, 48238, 25006, 42202, 9339, 3799, 24052, 42338,
    16516, 31847, 44579, 23884, 11253, 44392, 43512, 27505, 27197, 45982, 24671, 9692, 14520, 1352, 31311, 6780,
    27541, 42816, 42611, 45157, 7657, 17521, 41077, 5876, 21536, 8821, 10097, 40181, 31710, 13723, 19053, 31278,
    33824, 41579, 5857, 30739, 3799, 9835, 26222, 20444, 40865, 370, 28487, 11518, 16984, 15753, 27695, 39071,
    23682, 30014, 12778, 37345, 38913, 26623, 31285, 25227, 2278, 25009, 18461, 38035, 1165, 14602, 16211, 33302,
    13784, 41524, 44222, 19716, 27400, 47367, 43021, 45288, 25354, 24557, 27809, 674, 38913, 16303, 23337, 27701,
    19782, 35894, 41579, 49068, 26104, 25396, 43329, 11405, 24773, 2278, 40434, 18263, 1165, 36832, 9817, 9749,
    9943, 24354, 12239, 37345, 45982, 19801, 9021, 18074, 24052, 42468, 28665, 45288, 25796, 38871, 21186, 17722,
    23269, 18237, 31970, 24028, 5266, 33735, 41608, 42897, 15846, 43647, 37790, 42569, 39815, 674, 28418, 1352,
    36336, 11573, 25507, 45926, 24611, 42878, 45157, 31255, 37731, 11209, 19116, 22756, 48360, 40985, 42483, 41546,
    17701, 42700, 16598, 10778, 48545, 7523, 43590, 7324, 16411, 37354, 21001, 43067, 13650, 38913, 47875, 11393,
    28221, 18398, 31322, 27460, 30254, 29419, 11989, 17610, 14837, 14520, 47008, 9249, 24028, 35408, 42768, 40949,
    16697, 44637, 41883, 40181, 28308, 28241, 49135, 35325, 45761, 27617, 30464, 39205, 44060, 23767, 43314, 37969,
    3369, 18919, 9284, 25515, 17722, 21885, 28308, 26553, 31311, 31182, 28366, 9692, 17428, 9904, 40181, 29182,
    10188, 47964, 44117, 3369, 46678, 9278, 2174, 38913, 38819, 31311, 18237, 23142, 16270, 13723, 7311, 34216,
    9835, 45157, 19042, 24487, 28047, 23534, 40316, 15160, 25742, 17710, 10458, 13660, 44276, 21688, 15207, 2174,
    10265, 17957, 19953, 13421, 20729, 28723, 1287, 38973, 7121, 23989, 45982, 23337, 49340, 15698, 17626, 28279,
    27547, 26518, 33244, 2017, 11680, 17701, 37505, 26051, 28788, 49411, 43907, 39286, 5567, 9312, 30968, 46331,
    37468, 21063, 38574, 24773, 32989, 2017, 17188, 18458, 8494, 42823, 48471, 25006, 6161, 6277, 13421, 20868,
    6176, 6292, 35646, 47656, 31721, 24052, 23071, 31768, 31376, 18539, 17549, 39855, 13421, 29515, 38913, 38807,
    18217, 33067, 20407, 40316, 17425, 28495, 43915, 8230, 27460, 14837, 10579, 34234, 46321, 17557, 39885, 27197,
    49411, 23142, 19386, 23197, 2949, 44222, 47987, 42878, 28444, 46882, 48025, 42459, 817, 28110, 13332, 2510,
    49938, 4236, 47330, 39836, 10585, 17408, 44403, 2949, 42468, 11518, 2510, 4236, 28515, 30827, 29943, 42195,
    30636, 34412, 44760, 29495, 817, 45651, 11914, 13820, 48788, 41767, 45836, 17610, 30560, 12194, 42593, 27726,
    12270, 11155, 23666, 44093, 49071, 22284, 24162, 16118, 11491, 11386, 12305, 21186, 27734, 45606, 21854, 45491,
    48173, 8676, 42928, 47764, 24603, 34744, 49651, 26907, 2383, 31433, 48788, 43829, 15943, 16352, 26991, 31359,
    25969, 38157, 32617, 41014, 25354, 16436, 43453, 42202, 35142, 18799, 40236, 43808, 10006, 10234, 9094, 44231,
    11733, 7943, 29170, 29927, 28308, 24704, 20249, 40666, 1368, 6956, 32155, 13150, 20282, 40695, 8994, 27484,
    10987, 41978, 17454, 33682, 2853, 49411, 43345, 31903, 42085, 31137, 33575, 39609, 17668, 31632, 8420, 12659,
    30222, 27484, 32581, 10916, 39609, 29162, 8420, 49533, 45950, 1798, 8480, 17998, 35463, 38209, 27233, 38214,
    22538, 19033, 10480, 27828, 24947, 16436, 11820, 14022, 17764, 24757, 49611, 44980, 17408, 47802, 18717, 23184,
    34938, 27096, 22035, 49277, 4452, 44871, 40316, 19511, 42849, 43153, 47796, 21854, 23805, 27416, 26760, 9059,
    40919, 33987, 17035, 16387, 49833, 23325, 43594, 42224, 30300, 46904, 31649, 23459, 49593, 27804, 23142, 28571,
    49731, 16471, 46016, 37151, 41054, 38913, 8866, 8006, 14694, 38270, 785, 43383, 16534, 30254, 8015, 28252,
    47001, 10573, 4282, 40292, 38214, 36797, 16099, 41958, 34954, 9622, 30057, 44014, 11796, 13959, 2624, 19566,
    34379, 3577, 23586, 24401, 17963, 25009, 26359, 34216, 13571, 23337, 19386, 7602, 31917, 31930, 12071, 34579,
    11481, 19376, 46678, 24967, 2434, 49860, 29927, 26553, 3286, 33682, 24059, 49306, 34659, 19616, 29894, 45025,
    47811, 3857, 1697, 2434, 3286, 34362, 5408, 41205, 2624, 36200, 44760, 45157, 11253, 37757, 29215, 24671,
    24417, 10100, 8661, 11989, 15698, 86, 33893, 47155, 27715, 27720, 23979, 16598, 785, 10987, 19585, 8682,
    43259, 33928, 30350, 25644, 11865, 10329, 6780, 39139, 16598, 37463, 30345, 43383, 48294, 8961, 19463, 27460,
    16243, 47710, 38262, 8569, 19782, 10131, 32104, 41014, 44222, 47547, 10078, 21738, 22513, 40025, 28476, 49025,
    26745, 27016, 7561, 28047, 8522, 9477, 10006, 14873, 48181, 9504, 12123, 11649, 40351, 43329, 49860, 38913,
    27505, 38850, 9230, 30636, 20249, 40666, 23142, 16418, 33373, 1476, 41724, 9794, 31953, 7495, 45157, 49221,
    24052, 8412, 15584, 43305, 18636, 10381, 15207, 44626, 43210, 18662, 17764, 16640, 38402, 19386, 1948, 28038,
    9892, 33212, 45813, 37100, 11055, 2756, 11064, 45157, 41511, 2609, 16598, 7169, 22611, 19501, 14123, 8098,
    31351, 25035, 28308, 27629, 3324, 5671, 16231, 6923, 15846, 40915, 44692, 25734, 25219, 25120, 18781, 46063,
    19504, 2803, 30456, 40321, 30300, 13256, 22989, 10599, 17695, 3324, 40762, 36368, 14367, 8486, 15846, 17015,
    24732, 1758, 46793, 23142, 14683, 11443, 36564, 21576, 44222, 4392, 48428, 42292, 41148, 28030, 41617, 1476,
    27490, 43594, 12321, 31261, 26623, 1948, 29765, 5635, 46391, 18781, 21809, 8858, 12157, 26984, 41205, 24671,
    23142, 22799, 33049, 44930, 2756, 2609, 31137, 23142, 20035, 40402, 36576, 18580, 23080, 40316, 7482, 4351,
    39585, 29419, 23537, 13888, 23800, 42182, 4660, 2803, 16984, 23174, 24671, 12887, 12312, 3019, 7278, 45942,
    12669, 21423, 10265, 49508, 6988, 46219, 14588, 40181, 23096, 18525, 26184, 21567, 49706, 5314, 21172, 36171,
    43829, 29744, 15846, 45157, 46838, 29903, 5113, 17891, 24765, 14123, 25480, 5610, 49995, 36518, 9038, 3729,
    30555, 29453, 37401, 38425, 27490, 23210, 36271, 7961, 5039, 26235, 14433, 34216, 65535, 6010, 10273, 21971,
    21008, 43553, 2853, 18848, 10941, 45288, 12692, 48582, 6688, 30508, 10025, 9719, 36971, 13845, 4282, 27848,
    30636, 34839, 16317, 17704, 26538, 10035, 49320, 36502, 3729, 39015, 25379, 13801, 28542, 33049, 46263, 49774,
    22118, 27460, 23989, 24671, 29419, 22551, 35684, 24957, 12548, 38900, 9943, 17068, 11699, 17750, 23142, 45786,
    43640, 29490, 26518, 35359, 37446, 38291, 46860, 41215, 26596, 10107, 24844, 3473, 10436, 2772, 7785, 14715,
    22007, 29980, 43654, 49071, 1872, 14977, 46391, 2119, 16598, 10778, 2383, 14895, 48471, 21072, 48099, 1907,
    22722, 40549, 18051, 2965, 3927, 3763, 30636, 21581, 28320, 18413, 33973, 29992, 3473, 21704, 1798, 3927,
    25147, 33281, 19759, 22524, 25624, 32371, 44222, 24052, 48025, 30821, 28110, 11518, 21536, 9528, 43829, 22739,
    1586, 19463, 31947, 1219, 26013, 27197, 19386, 40809, 39402, 22300, 49792, 17127, 40256, 40809, 29590, 18237,
    47829, 34759, 43873, 1613, 43775, 46239, 3389, 8522, 16598, 8188, 9990, 49576, 46325, 43863, 1014, 6365,
    11542, 23646, 21659, 21817, 46194, 1613, 4710, 2348, 13571, 3389, 11089, 20202, 1014, 48418, 16352, 38913,
    1069, 12989, 32136, 16984, 16093, 16280, 23848, 23142, 3228, 33330, 23926, 49381, 35342, 21008, 8162, 46734,
    47099, 12486, 40280, 24478, 41793, 14271, 11829, 35125, 28476, 47945, 15846, 33911, 25348, 45462, 24935, 26074,
    33420, 45915, 4332, 27977, 3421, 981, 21854, 33497, 22638, 7750, 12001, 26806, 37969, 25354, 29698, 14812,
    38414, 30514, 49684, 5384, 38480, 38606, 24671, 23337, 12962, 14724, 41793, 34971, 33751, 10746, 31295, 11905,
    21406, 40639, 1490, 4904, 41711, 31563, 31970, 21375, 35495, 14446, 36849, 31632, 9860, 7513, 33049, 16228,
    24871, 39050, 16426, 8210, 1069, 44222, 10424, 43829, 3491, 45982, 31443, 1490, 32847, 25566, 35342, 9835,
    3783, 28773, 42468, 13755, 7105, 36939, 9312, 49692, 4139, 12254, 2788, 8300, 44565, 9151, 45455, 18398,
    11361, 46336, 19323, 8547, 49576, 13614, 11201, 43453, 49201, 26623, 4139, 2788, 41686, 34216, 17541, 3078,
    981, 35511, 47073, 16436, 3078, 15522, 17454, 34938, 11594, 46634, 16551, 36305, 4211, 30071, 27311, 4332,
    12477, 46140, 37556, 3421, 981, 1524, 14149, 7406, 15434, 34579, 47112, 21979, 4260, 4211, 30136, 41410,
    13828, 20982, 41904, 1524, 876, 49964, 35359, 28470, 17276, 16231, 48360, 40212, 22971, 37543, 49108, 39116,
    46826, 1108, 11989, 18263, 12071, 34579, 41703, 41555, 4260, 9692, 35443, 19129, 31463, 3491, 8210, 876,
    39965, 26518, 35024, 3892, 21545, 3943, 19386, 49747, 32908, 4192, 1997, 19504, 3943, 8602, 34426, 37987,
    16582, 14531, 2348, 3892, 4192, 34559, 28397, 47675, 11820, 15698, 17428, 25977, 16939, 39188, 45533, 17715,
    33778, 44222, 6058, 4192, 1997, 46606, 41166, 11989, 30916, 35125, 26112, 8168, 21414, 21949, 43654, 12173,
    50009, 11644, 48448, 1586, 24496, 42807, 1219, 23197, 47861, 45359, 11899, 18272, 8068, 45348, 49490, 10778,
    27381, 17610, 35085, 12298, 16598, 24557, 8591, 37345, 19716, 46246, 4088, 46308, 6786, 28094, 24188, 43389,
    24582, 31721, 3189, 49403, 23767, 48360, 19214, 37342, 16598, 43938, 10006, 24557, 30329, 43449, 41378, 46674,
    18279, 15556, 3562, 18509, 22026, 21879, 43329, 7454, 49411, 24293, 44080, 2451, 19782, 34395, 44222, 4088,
    19205, 22253, 21008, 29387, 41546, 30417, 42092, 43079, 49740, 42213, 17891, 48300, 16411, 3562, 11321, 23906,
    17722, 30724, 18398, 2915, 31311, 40878, 25227, 9653, 22258, 12184, 2451, 46279, 36106, 23104, 3189, 49876,
    28205, 16590, 39940, 21178, 10143, 46146, 49792, 12802, 11518, 9883, 30372, 46028, 44766, 26275, 15949, 19243,
    27788, 48627, 17652, 11424, 17738, 23153, 15805, 49002, 15546, 10592, 44003, 13436, 39382, 30806, 45982, 12962,
    38562, 49568, 18944, 23184, 16188, 8764, 45976, 10188, 38249, 15540, 39926, 11089, 9275, 13723, 37715, 40190,
    49533, 17294, 25954, 38024, 40196, 12092, 17428, 31801, 47119, 32480, 10516, 44222, 12028, 45157, 24626, 17135,
    47360, 13246, 8967, 8936, 48025, 18557, 13381, 10006, 1719, 31113, 11448, 14801, 49062, 349, 15448, 9323,
    21186, 2325, 14278, 44174, 38913, 11690, 37707, 31480, 3355, 38013, 24373, 41156, 23337, 20991, 11670, 28571,
    47268, 28258, 10134, 16211, 14913, 36234, 31203, 15846, 24258, 19701, 42878, 31272, 27720, 16169, 12669, 48285,
    41893, 44164, 37523, 21854, 22698, 38913, 413, 44222, 12443, 15846, 29481, 43829, 28308, 9544, 26663, 46321,
    30156, 28542, 13074, 27547, 8428, 29509, 23142, 27016, 49350, 40181, 33359, 44222, 31698, 9051, 24052, 8967,
    18636, 10461, 24671, 16873, 9874, 42660, 22713, 14804, 43145, 18086, 27311, 29501, 15746, 45150, 18557, 39027,
    18781, 20122, 10778, 22840, 49178, 23467, 22660, 41402, 45259, 25796, 48497, 41686, 10631, 4488, 49477, 19463,
    44507, 22380, 41370, 8732, 41904, 41024, 16070, 31424, 24363, 37589, 17750, 11670, 10445, 7684, 41200, 5363,
    9874, 12832, 34615, 12788, 19307, 44661, 22660, 15007, 42563, 22351, 38299, 12423, 43741, 38414, 20138, 18572,
    30995, 22698, 44174, 28997, 12329, 48044, 43267, 24704, 2592, 1813, 16901, 1425, 49517, 35024, 23773, 14514,
    19307, 45157, 23113, 25711, 42843, 13778, 21933, 13184, 29733, 39754, 7387, 39468, 40316, 8494, 21536, 25742,
    19855, 7332, 26104, 15448, 24327, 9323, 15974, 47303, 21032, 41038, 43430, 15986, 38913, 30360, 28418, 31223,
    19386, 28038, 9059, 27223, 19782, 32249, 10451, 37977, 22399, 31384, 15085, 22059, 37454, 25723, 19701, 30760,
    39675, 2592, 31509, 19830, 31606, 42499, 32869, 27104, 26957, 15846, 45157, 45642, 22016, 11865, 42583, 44671,
    8801, 18288, 29401, 39855, 49257, 44717, 15840, 13650, 38913, 9995, 27197, 27701, 15614, 29470, 38150, 13015,
    16794, 16016, 16775, 44455, 21148, 41546, 40474, 45288, 16788, 44618, 9579, 12995, 13002, 8138, 22919, 17168,
    23989, 15698, 14433, 33156, 46194, 22647, 37888, 23482, 17803, 24444, 43298, 26266, 42385, 20917, 22026, 37822,
    15272, 29765, 15044, 1368, 16752, 37100, 45157, 47035, 41739, 49490, 8967, 42092, 30268, 45288, 17330, 25728,
    41686, 46402, 43863, 24914, 49257, 23960, 48081, 24671, 9692, 28523, 30791, 44222, 26916, 10621, 25644, 38005,
    21872, 42483, 11294, 49825, 2837, 21536, 29053, 11089, 16820, 25243, 16441, 31649, 3060, 24757, 29106, 23989,
    31285, 29024, 39530, 34691, 8977, 1388, 31571, 8383, 34305, 46134, 19129, 11841, 14642, 42878, 30941, 30706,
    44645, 41344, 9237, 29336, 27720, 41546, 39707, 8494, 12579, 24590, 19822, 29951, 17952, 22190, 18098, 42849,
    17159, 18129, 31500, 37454, 46435, 27245, 41904, 30024, 13711, 45625, 46454, 3060, 25933, 25818, 26553, 30254,
    39092, 4762, 18263, 26876, 23235, 33702, 37454, 26663, 44356, 30505, 40997, 18610, 31555, 24757, 32721, 24490,
    14650, 11429, 47398, 25354, 25796, 12050, 28542, 18601, 29776, 9799, 46851, 23482, 31623, 12669, 39745, 22275,
    38799, 17529, 22351, 29789, 10760, 22319, 44988, 42010, 37345, 19398, 7463, 19773, 39066, 18398, 28418, 9692,
    11989, 31970, 25009, 49191, 12919, 44222, 43800, 43092, 19463, 10424, 39754, 2837, 17044, 23467, 23034, 13769,
    15367, 22140, 29401, 37959, 40346, 21933, 26257, 10396, 16984, 41686, 42213, 16483, 17817, 12162, 21788, 29082,
    42414, 43113, 12756, 22179, 21825, 11429, 10885, 30505, 15160, 10550, 48545, 21605, 11570, 47259, 46413, 39855,
    47841, 11018, 14520, 27223, 36219, 19444, 37987, 21429, 3172, 42878, 1201, 16886, 39871, 11491, 47889, 7544,
    30666, 31733, 26456, 42639, 18372, 10691, 23219, 22602, 3355, 14520, 32829, 1201, 12746, 15590, 41989, 49313,
    19281, 44392, 22906, 23906, 9719, 27440, 24704, 9692, 17610, 14520, 40136, 25742, 44222, 10746, 44602, 19096,
    22351, 39717, 31396, 45415, 38913, 49089, 22337, 18237, 31285, 30902, 35105, 37757, 38364, 32386, 1388, 49869,
    42468, 29733, 8821, 17582, 1719, 19687, 11448, 49068, 37870, 18299, 26553, 16873, 32974, 44222, 17492, 13405,
    13985, 2325, 23142, 34079, 11379, 20947, 13571, 13544, 26310, 46597, 16377, 43161, 17529, 43950, 29401, 22704,
    25200, 37698, 22792, 41904, 14588, 44725, 41767, 24671, 14837, 10296, 45229, 25610, 22859, 34530, 28303, 25787,
    31016, 37996, 12279, 11448, 30508, 15085, 21941, 12461, 42195, 37454, 7906, 44511, 31872, 38997, 11670, 27701,
    14942, 36186, 9778, 44222, 19636, 11865, 30071, 47198, 30981, 8967, 24977, 49490, 41227, 29429, 25988, 16169,
    23325, 28681, 6908, 43526, 17529, 43863, 44392, 31057, 42195, 41692, 21854, 15683, 38913, 21590, 3153, 24671,
    31285, 4640, 28754, 7053, 1976, 28523, 43465, 25009, 23235, 32974, 19444, 3694, 15691, 21642, 45321, 28506,
    43962, 3269, 3153, 1976, 28523, 43259, 34287, 26315, 46007, 859, 13582, 25354, 28965, 23989, 35392, 9082,
    11573, 999, 34201, 42878, 859, 23688, 43430, 18898, 13553, 16873, 999, 42934, 36127, 15829, 26138, 9698,
    47277, 42468, 43608, 24977, 3307, 27076, 11644, 2662, 48018, 10097, 41686, 3454, 22351, 28937, 644, 29401,
    46658, 11542, 27848, 17182, 14206, 2152, 28308, 21590, 26553, 18237, 38312, 29419, 16873, 21893, 19782, 5771,
    42611, 10880, 31071, 9979, 43229, 49071, 27765, 44913, 14416, 28308, 23142, 47141, 46367, 2647, 17846, 12629,
    27720, 22704, 44703, 23906, 2152, 644, 8284, 11036, 34823, 47230, 21758, 44518, 42028, 3454, 31549, 27838,
    7817, 14337, 24704, 39847, 19967, 34500, 13068, 24052, 6726, 8646, 42483, 12802, 19607, 38364, 45982, 12572,
    42499, 33529, 39015, 31071, 8494, 23337, 43259, 33017, 28924, 15123, 28866, 28038, 44276, 44970, 44476, 3131,
    46658, 11542, 23791, 47432, 14520, 35271, 27604, 11253, 49411, 9786, 35994, 19129, 3694, 45188, 12941, 3131,
    3269, 42634, 37781, 34065, 17105, 26772, 28308, 25009, 43492, 42878, 5059, 16598, 47583, 43594, 2647, 32800,
    42103, 44653, 26266, 23142, 39517, 33067, 14367, 25644, 26035, 10006, 22660, 19840, 45873, 1813, 1425, 34170,
    13874, 12854, 22359, 24052, 16009, 12537, 43970, 20575, 31045, 12867, 24671, 31285, 17408, 35125, 23290, 37935,
    20763, 31890, 19173, 44717, 23256, 28981, 23895, 44222, 15829, 26683, 2135, 41546, 17722, 37543, 34780, 42978,
    29754, 37912, 15919, 2135, 8944, 14970, 10823, 46793, 22066, 21688, 31641, 22059, 30873, 42576, 41484, 21681,
    19866, 23331, 38913, 26670, 19920, 27163, 7463, 11804, 19879, 23269, 23337, 27547, 19782, 37085, 15829, 19636,
    42878, 25644, 23071, 44185, 47648, 49533, 48025, 11518, 30893, 21121, 25796, 17529, 14703, 31057, 49724, 12816,
    24898, 41054, 48664, 47057, 48379, 27505, 14337, 42166, 25809, 20991, 9662, 14520, 14520, 48386, 19830, 22672,
    43259, 32425, 44222, 14030, 9852, 17645, 15829, 42878, 7356, 47287, 16598, 20218, 14553, 43453, 44988, 38913,
    28308, 21590, 23142, 19386, 11989, 12209, 24028, 33103, 15716, 43794, 29495, 15440, 24052, 12669, 10100, 23688,
    45553, 47249, 37454, 19701, 28308, 26485, 23482, 47198, 10006, 11224, 591, 32293, 44222, 12028, 6780, 7710,
    17529, 11253, 24717, 15897, 26633, 23142, 1368, 22151, 35253, 31960, 47535, 13468, 10468, 41546, 23688, 47316,
    43741, 44988, 22893, 31311, 17428, 35178, 14367, 37987, 18306, 8316, 25796, 41686, 1734, 23337, 16952, 35837,
    11841, 8400, 20816, 41200, 37212, 25753, 42092, 740, 41424, 35976, 14367, 24052, 21698, 43220, 19701, 38913,
    26658, 29538, 39855, 33343, 49144, 2215, 10885, 22389, 30307, 10409, 17877, 3978, 22859, 36354, 11611, 18247,
    7934, 47569, 18547, 2662, 41364, 38652, 38799, 18279, 44036, 14822, 26304, 47390, 44030, 24691, 27460, 11922,
    38596, 31526, 41460, 49373, 35631, 8022, 13912, 14508, 37395, 16016, 42878, 26133, 22359, 23475, 21668, 39370,
    28168, 27178, 23080, 38691, 23482, 46462, 47277, 7561, 13888, 38546, 8027, 10100, 27541, 25728, 18636, 25062,
    26871, 22405, 24830, 12810, 17830, 31500, 43594, 41943, 43688, 23281, 13222, 25933, 30960, 41767, 23337, 26170,
    31285, 23142, 26196, 14520, 13723, 47602, 22309, 25009, 11036, 32943, 44222, 26157, 31687, 25920, 43573, 29336,
    24977, 49952, 18781, 44317, 16598, 40316, 19333, 10006, 22369, 47193, 14859, 9125, 12537, 27695, 29215, 7570,
    37454, 7724, 29643, 19701, 39271, 15044, 17929, 21958, 25933, 43362, 27197, 45982, 31285, 23142, 19386, 9692,
    16873, 14520, 19830, 31674, 23862, 43259, 35160, 47484, 15253, 47167, 3307, 30041, 34049, 48569, 48528, 44222,
    10006, 25087, 28308, 14520, 21581, 49135, 32176, 24905, 10490, 24704, 23337, 37945, 11670, 26323, 35286, 10669,
    39364, 39355, 48508, 19636, 23311, 16489, 24222, 8420, 28815, 7184, 28136, 24052, 31272, 22748, 15857, 21698,
    37813, 41546, 47936, 11660, 18509, 9029, 25515, 30222, 11125, 14822, 28155, 39382, 31710, 28308, 31311, 18237,
    25227, 9461, 17610, 14433, 27560, 44143, 40196, 15504, 33120, 16456, 23071, 15194, 20071, 7643, 30492, 38832,
    15818, 27296, 45720, 31311, 45836, 20975, 18761, 19315, 11726, 31571, 31563, 41904, 19701, 37680, 28000, 37767,
    22258, 26112, 37566, 37857, 16446, 39754, 8661, 3978, 33617, 7851, 19323, 18404, 2233, 19985, 40358, 42528,
    15974, 24757, 20264, 40679, 33405, 21106, 30345, 2476, 25103, 37628, 39570, 39619, 33552, 19687, 11448, 38312,
    32784, 41865, 23688, 2476, 43633, 20991, 34216, 16511, 25299, 27918, 25650, 25456, 35232, 46882, 28886, 7233,
    7737, 44373, 41205, 15097, 25250, 42855, 8437, 13264, 43389, 18247, 23165, 29733, 19680, 12737, 25903, 29189,
    13126, 9211, 8828, 8614, 42405, 16984, 8661, 42414, 28308, 28038, 45707, 16211, 36449, 45170, 42213, 29144,
    42885, 25333, 42873, 42074, 29319, 1546, 23666, 25753, 37473, 24049, 20675, 18636, 48788, 47525, 18062, 45594,
    22722, 5752, 49209, 13670, 44997, 27060, 19059, 23916, 19069, 46941, 43, 20559, 20799, 19386, 49108, 45836,
    15698, 39907, 47802, 14602, 21304, 34463, 13784, 7073, 17919, 25644, 38617, 29631, 44276, 21195, 18357, 41205,
    17929, 10987, 40248, 19109, 47602, 48774, 33595, 49076, 18991, 44222, 25238, 31085, 1546, 18885, 21356, 23666,
    27893, 13633, 29580, 22399, 4106, 44734, 41546, 19847, 30245, 10006, 43896, 29602, 25389, 43298, 9278, 11253,
    42849, 40280, 39836, 8034, 13793, 21799, 18357, 14822, 22118, 8121, 38769, 28308, 46941, 13678, 9692, 21326,
    41817, 27223, 6762, 42446, 42753, 37578, 43259, 32205, 25094, 15829, 25080, 49396, 7317, 15023, 16040, 11934,
    24549, 38799, 18950, 44392, 1907, 14427, 38973, 713, 24704, 49108, 18944, 47602, 35927, 41910, 41899, 22962,
    24741, 275, 8129, 12669, 5961, 12659, 15974, 15631, 38913, 619, 28038, 40942, 315, 41134, 8532, 44222,
    45479, 49154, 14514, 556, 13107, 188, 22413, 29602, 26615, 12659, 41686, 8074, 22052, 43594, 1907, 45982,
    24028, 111, 25693, 48401, 9069, 20947, 49980, 43654, 25243, 37454, 20729, 25162, 15044, 45982, 16873, 16952,
    47602, 12134, 11850, 22859, 34823, 27677, 13784, 29374, 29361, 38755, 48371, 24052, 40961, 24258, 30716, 46725,
    12649, 15698, 19716, 19737, 43505, 12941, 38048, 45963, 23142, 9692, 16952, 14520, 31970, 35715, 44712, 14048,
    21758, 15829, 2215, 23014, 41546, 44497, 11089, 25933, 38444, 17428, 10368, 14053, 2527, 34598, 2233, 22435,
    12659, 41004, 22722, 14337, 48957, 42283, 16175, 35359, 12113, 41874, 43647, 11424, 14520, 38632, 36888, 39933,
    16139, 7534, 49221, 28476, 25424, 44133, 42468, 26907, 41546, 31384, 19504, 14454, 15818, 43829, 15974, 21941,
    41835, 39200, 28378, 38913, 10531, 6874, 38583, 14337, 23989, 46208, 22950, 28651, 47602, 6753, 8040, 20342,
    9799, 44246, 38541, 14354, 15829, 9835, 13641, 48204, 4432, 4467, 5460, 38378, 25044, 770, 31336, 23411,
    11542, 40785, 37454, 39226, 23557, 21590, 5136, 23269, 31311, 23142, 11989, 28038, 9249, 23235, 10134, 19782,
    33651, 42679, 46420, 23998, 21902, 43478, 26343, 26663, 25845, 8953, 39543, 12104, 11865, 46231, 25850, 25801,
    46114, 14014, 45634, 770, 9767, 39815, 46429, 48418, 11536, 43220, 31555, 27136, 24757, 29353, 27460, 23989,
    9075, 15380, 18944, 22151, 15434, 16908, 12020, 42878, 22941, 21649, 37800, 42700, 19709, 10709, 14588, 23838,
    38973, 17722, 28016, 37543, 24671, 6859, 42283, 35851, 44222, 15829, 13325, 37317, 15194, 10718, 10676, 20669,
    42321, 8494, 10006, 6096, 17513, 43594, 40181, 19413, 49249, 24671, 9692, 14520, 49462, 9339, 15060, 25187,
    5826, 44222, 41452, 26843, 28397, 42978, 14223, 44511, 37345, 26855, 26828, 38395, 4996, 29865, 36732, 10342,
    21698, 20526, 8151, 29236, 12506, 17877, 14992, 30172, 29210, 43505, 43229, 47516, 47360, 42114, 41546, 10906,
    42132, 15180, 8953, 25315, 12604, 16024, 28288, 11526, 12501, 32011, 15398, 14123, 23791, 48340, 47607, 40203,
    23142, 29401, 41090, 40930, 23403, 38926, 29010, 12659, 43594, 8174, 38194, 37337, 20703, 29765, 29395, 25130,
    13734, 22602, 43763, 47841, 27701, 28038, 42357, 3673, 35563, 22962, 45157, 22731, 24977, 47563, 45077, 31656,
    15936, 46585, 21799, 29353, 11922, 49659, 16211, 35548, 8393, 41248, 42182, 27878, 44128, 38729, 12767, 38224,
    28083, 7796, 15829, 14514, 26287, 26532, 47630, 38696, 27431, 40997, 24424, 6148, 6549, 6264, 6082, 6447,
    6352, 6487, 6573, 6644, 12659, 38048, 48750, 16295, 12501, 22026, 48135, 16713, 21996, 25364, 25638, 23505,
    27136, 43100, 10496, 27586, 38395, 8742, 38122, 31406, 47841, 26553, 37543, 24671, 45402, 43422, 38714, 17237,
    38035, 46038, 45895, 31137, 15111, 47218, 15846, 49361, 47277, 41546, 38425, 16598, 7305, 6657, 6386, 14740,
    27560, 23654, 4370, 47205, 31128, 39445, 30382, 19529, 19520, 26127, 7619, 15233, 29810, 38386, 13107, 46753,
    27044, 10778, 38742, 38680, 49551, 27672, 24557, 24025, 39774, 29789, 44717, 43546, 15459, 25155, 16361, 26121,
    8661, 13317, 46766, 38142, 40624, 41460, 7376, 27472, 35479, 49221, 13985, 47602, 42611, 45715, 26883, 45533,
    25753, 43654, 31002, 38496, 8816, 11424, 15818, 43829, 11253, 8368, 24704, 12962, 15493, 35580, 44222, 9749,
    7785, 13297, 29529, 43298, 8519, 9278, 43397, 45348, 10644, 23317, 30536, 13650, 37345, 9426, 38913, 31710,
    30748, 28308, 26623, 26553, 45982, 46208, 23337, 23142, 25227, 14520, 14433, 13723, 17428, 25009, 37432, 32738,
    19457, 44810, 41344, 24977, 12082, 22399, 19789, 38670, 44962, 38913, 15044, 38469, 26553, 23269, 31285, 9692,
    15698, 27701, 23862, 35531, 19129, 37649, 44222, 16582, 45851, 16222, 46287, 16598, 10718, 12612, 10204, 10987,
    27460, 23989, 23337, 23142, 15698, 14837, 25009, 18461, 46559, 12514, 11989, 19444, 21339, 49651, 4106, 48556,
    29223, 18939, 43453, 48418, 13793, 17610, 8816, 9296, 25489, 42878, 39880, 23137, 18428, 6845, 4958, 17529,
    11578, 39243, 18398, 6975, 42432, 33182, 27815, 44222, 19667, 49869, 13272, 21758, 26663, 15317, 2609, 45205,
    7264, 28636, 6816, 36677, 20653, 20851, 45054, 23688, 38291, 8944, 42109, 25864, 19673, 14743, 11573, 42849,
    42195, 25869, 47979, 5701, 11536, 25522, 40707, 44863, 38082, 40181, 38913, 41358, 44044, 3209, 25933, 28308,
    26623, 27460, 31311, 23989, 24293, 20185, 22258, 40514, 45059, 7477, 18944, 27903, 9898, 43259, 37012, 29302,
    44970, 37649, 46573, 23311, 21008, 16078, 17302, 46070, 28118, 8420, 25943, 21356, 40654, 24052, 47665, 37687,
    9552, 40301, 6317, 6461, 6523, 6609, 6671, 6201, 6400, 21517, 40593, 8494, 12669, 6068, 39815, 11660,
    43594, 41495, 44511, 37345, 38913, 38061, 4413, 5654, 28832, 26553, 37543, 23989, 23142, 9692, 1887, 14837,
    25577, 25009, 40610, 29293, 11583, 33227, 2998, 31995, 23716, 38386, 36070, 29309, 28594, 29325, 39015, 2998,
    7219, 16503, 29336, 16222, 27016, 13054, 10100, 48310, 21536, 497, 14623, 31500, 11542, 42551, 28308, 31311,
    24671, 23337, 1887, 27004, 49135, 13044, 34273, 15406, 37515, 49052, 22962, 38005, 18036, 7411, 21276, 27886,
    39175, 17521, 18323, 23494, 6303, 6509, 6595, 6657, 27695, 46522, 48968, 28191, 18398, 23989, 16873, 27223,
    233, 21172, 16752, 37199, 43041, 45157, 21668, 26493, 14261, 12501, 14588, 14255, 22815, 27136, 11107, 28308,
    47471, 45068, 26855, 41760, 39795, 15829, 42978, 22359, 7018, 24626, 39783, 29687, 43287, 49477, 45696, 45123,
    14822, 23827, 45085, 18350, 12531, 44419, 47442, 41999, 11989, 37041, 34677, 15829, 39348, 21008, 45157, 45157,
    46709, 21276, 27866, 29825, 15356, 21698, 12298, 44022, 17704, 41090, 24643, 24683, 25405, 22739, 24209, 16161,
    44299, 37654, 46678, 21038, 24651, 15913, 19299, 38913, 23989, 45836, 14520, 17428, 14102, 49135, 14560, 46080,
    30437, 33034, 9331, 9852, 15605, 40732, 24075, 10787, 18670, 27924, 42878, 15846, 26066, 21318, 31721, 11979,
    11141, 18841, 3019, 9943, 24977, 24080, 4828, 48459, 27777, 12669, 24354, 10006, 25963, 8944, 48453, 17891,
    42790, 49918, 39815, 573, 18678, 43594, 20737, 48211, 43727, 40181, 37345, 48323, 25933, 47841, 46941, 19471,
    23142, 48465, 16915, 21510, 2364, 42915, 18832, 15728, 23697, 42878, 16052, 29387, 6046, 8591, 26104, 6636,
    44392, 26184, 31267, 18813, 38280, 10160, 22989, 14837, 10134, 7347, 41110, 24957, 20313, 23938, 48392, 17090,
    38940, 9913, 20001, 40372, 48159, 14495, 48256, 37454, 9951, 17025, 30578, 28229, 18944, 23646, 9606, 44363,
    44222, 26138, 20947, 44216, 30878, 41511, 9698, 16107, 38662, 45523, 9712, 45288, 15734, 37496, 48018, 10003,
    12363, 8262, 13421, 27281, 23337, 43640, 29851, 37048, 17790, 48643, 9706, 45802, 30407, 39301, 43136, 10461,
    30827, 9373, 22630, 45982, 24671, 24657, 12962, 19026, 33073, 19077, 29569, 11979, 9138, 29410, 49651, 24276,
    12202, 38641, 27756, 8944, 28174, 24557, 43741, 43083, 31744, 40181, 13650, 38913, 30202, 46941, 21590, 27460,
    31285, 11989, 14520, 12043, 48803, 19960, 19009, 14439, 24230, 30191, 10134, 32707, 43436, 24239, 3816, 23120,
    17704, 10198, 16311, 30335, 16598, 12669, 30345, 49068, 9519, 18516, 38048, 44463, 45077, 11253, 25250, 25179,
    37638, 20745, 2876, 23242, 44790, 26553, 18237, 31597, 33944, 44222, 18337, 11352, 3816, 32021, 4507, 46070,
    44810, 20637, 20834, 31617, 12312, 25753, 21284, 42483, 38546, 165, 14615, 13598, 43305, 9195, 49265, 48418,
    48135, 30307, 37345, 38913, 9719, 23337, 31453, 46537, 44107, 27025, 24890, 20592, 15846, 9043, 27210, 27460,
    31606, 43707, 23142, 29840, 41176, 10131, 11193, 2876, 26623, 11989, 36469, 9676, 17541, 18980, 21096, 9835,
    24095, 28625, 28444, 49221, 26222, 21698, 26310, 9015, 24258, 46051, 44777, 30395, 6737, 15818, 24170, 9278,
    13985, 6325, 10265, 24478, 42781, 40265, 38955, 29064, 29134, 24704, 24671, 23142, 27547, 35596, 40275, 38091,
    45157, 44692, 40103, 48993, 17564, 4741, 9354, 16200, 12537, 46491, 10732, 18129, 18662, 11405, 14962, 27256,
    45982, 46941, 31311, 29419, 14520, 21161, 17428, 12396, 21437, 32234, 46471, 39682, 15927, 28695, 11411, 15652,
    31937, 38425, 21083, 46165, 30508, 42213, 15936, 7915, 49484, 28308, 39484, 32446, 7626, 9930, 25644, 11743,
    11778, 30548, 17428, 21128, 35427, 11676, 15829, 48716, 11215, 42422, 21698, 9382, 21050, 12230, 15659, 44299,
    11820, 6334, 6411, 6469, 6531, 28436, 18357, 46379, 28038, 33481, 11773, 17704, 10044, 18362, 43259, 16697,
    44222, 18657, 12348, 24052, 26222, 12263, 45278, 537, 20328, 40773, 28462, 12589, 9868, 17573, 23505, 27701,
    28453, 33959, 448, 13784, 65535, 49869, 19103, 18686, 43583, 15569, 472, 6119, 6235, 24923, 10281, 48269,
    44446, 49910, 10523, 21393, 32645, 9773, 11281, 12370, 9647, 9959, 7926, 15207, 25250, 45821, 11542, 37454,
    21001, 23337, 37945, 23142, 9874, 10566, 33197, 5590, 24822, 24749, 6128, 6244, 14731, 44209, 18824, 49602,
    39955, 21456, 21467, 21205, 22351, 29789, 21445, 39460, 15419, 29082, 19398, 44203, 12495, 16129, 2364, 21297,
    251, 4543, 24531, 22246, 21222, 24085, 24520, 45890, 21232, 11637, 17311, 49229, 18628, 6742, 21387, 9160,
    48232, 37628, 40171, 26026, 48226, 49782, 15334, 18500, 39552, 20939, 17356, 28563, 48443, 9184, 48220, 39080,
    21333, 19002, 9172, 24525, 32755, 19016, 20049, 23749, 49845, 26035, 17207, 12389, 16598, 21241, 10244, 8555,
    6679, 20751, 6377, 6411, 42710, 23906, 16557, 23337, 10134, 45776, 34516, 47230, 10831, 15710, 26470, 25717,
    42156, 23377, 24035, 20933, 11276, 25594, 15207, 6438, 6500, 6586, 6627, 14406, 41835, 19260, 12934, 15698,
    24471, 41424, 8316, 14998, 40414, 30649, 12413, 24181, 42878, 9968, 28174, 9399, 31817, 20515, 11253, 6429,
    21499, 46550, 12461, 18357, 23242, 7382, 18362, 39393, 27355, 49411, 16952, 21112, 17600, 31780, 17428, 9339,
    48164, 35216, 7611, 11865, 24565, 24258, 19129, 12270, 21008, 17119, 24490, 44802, 23518, 13748, 39603, 12353,
    45982, 16243, 36289, 24042, 12443, 16572, 45881, 33003, 15974, 40450, 37064, 44222, 28059, 25306, 11979, 18781,
    9487, 8577, 24557, 9579, 9278, 16952, 14520, 31970, 7839, 42945, 11302, 39436, 16598, 23947, 25781, 21590,
    27472,
};

constexpr unsigned kNumProv = 7;
 
inline std::string_view Nombre(NameOff o) {
    return o == kNone ? std::string_view{} : std::string_view(kNombres + o);
}
 
// Los indices devuelven -1 si el codigo no existe (fuera de rango o hueco).
inline int IdxProv(unsigned p) {
    return (p >= 1 && p <= kNumProv && kProvNombre[p - 1] != kNone) ? int(p - 1) : -1;
}
 
inline int IdxCanton(unsigned p, unsigned c) {
    if (p < 1 || p > kNumProv || c < 1) return -1;
    const unsigned ini = kProvCantonIni[p - 1], fin = kProvCantonIni[p];
    if (c > fin - ini) return -1;
    const unsigned i = ini + c - 1;
    return kCantonNombre[i] != kNone ? int(i) : -1;
}
 
inline int IdxDistrito(unsigned p, unsigned c, unsigned d) {
    const int ci = IdxCanton(p, c);
    if (ci < 0 || d < 1) return -1;
    const unsigned ini = kCantonDistIni[ci], fin = kCantonDistIni[ci + 1];
    if (d > fin - ini) return -1;
    const unsigned i = ini + d - 1;
    return kDistNombre[i] != kNone ? int(i) : -1;
}
 
// Cuenta / ubica elementos validos (sin hueco) dentro de [ini, fin).
inline uint32_t ContarValidos(const NameOff* nombres, unsigned ini, unsigned fin) {
    uint32_t n = 0;
    for (unsigned i = ini; i < fin; ++i) n += (nombres[i] != kNone);
    return n;
}
inline int NesimoValido(const NameOff* nombres, unsigned ini, unsigned fin, uint32_t n) {
    for (unsigned i = ini; i < fin; ++i)
        if (nombres[i] != kNone && n-- == 0) return int(i);
    return -1;
}
 
#if DT_CON_BARRIOS
inline int IdxBarrio(unsigned p, unsigned c, unsigned d, unsigned b) {
    const int di = IdxDistrito(p, c, d);
    if (di < 0 || b < 1) return -1;
    const unsigned ini = kDistBarrioIni[di], fin = kDistBarrioIni[di + 1];
    if (b > fin - ini) return -1;
    const unsigned i = ini + b - 1;
    return kBarrioNombre[i] != kNone ? int(i) : -1;
}
#endif
 
}  // namespace
 
bool DivisionTerritorial::DescomponerCodigoPostal(uint32_t cod, uint8_t& p, uint8_t& c, uint8_t& d, uint8_t& b) {
    if (cod < 1000000u || cod > 9999999u) return false;
    p = uint8_t(cod / 1000000u);
    c = uint8_t((cod / 10000u) % 100u);
    d = uint8_t((cod / 100u) % 100u);
    b = uint8_t(cod % 100u);
    return true;
}
 
bool DivisionTerritorial::EsProvinciaValida(uint8_t p) { return IdxProv(p) >= 0; }
bool DivisionTerritorial::EsCantonValido(uint8_t p, uint8_t c) { return IdxCanton(p, c) >= 0; }
bool DivisionTerritorial::EsDistritoValido(uint8_t p, uint8_t c, uint8_t d) { return IdxDistrito(p, c, d) >= 0; }
bool DivisionTerritorial::EsUbicacionValida(uint8_t p, uint8_t c, uint8_t d) { return IdxDistrito(p, c, d) >= 0; }
 
bool DivisionTerritorial::EsBarrioValido(uint8_t p, uint8_t c, uint8_t d, uint8_t b) {
#if DT_CON_BARRIOS
    return IdxBarrio(p, c, d, b) >= 0;
#else
    (void)p; (void)c; (void)d; (void)b;
    return false;
#endif
}
 
bool DivisionTerritorial::EsBarrioValido(uint32_t codigoPostal) {
    uint8_t p, c, d, b;
    return DescomponerCodigoPostal(codigoPostal, p, c, d, b) && EsBarrioValido(p, c, d, b);
}
 
std::string_view DivisionTerritorial::GetNombreProvincia(uint8_t p) {
    const int i = IdxProv(p);
    return i < 0 ? std::string_view{} : Nombre(kProvNombre[i]);
}
 
std::string_view DivisionTerritorial::GetNombreCanton(uint8_t p, uint8_t c) {
    const int i = IdxCanton(p, c);
    return i < 0 ? std::string_view{} : Nombre(kCantonNombre[i]);
}
 
std::string_view DivisionTerritorial::GetNombreDistrito(uint8_t p, uint8_t c, uint8_t d) {
    const int i = IdxDistrito(p, c, d);
    return i < 0 ? std::string_view{} : Nombre(kDistNombre[i]);
}
 
std::string_view DivisionTerritorial::GetNombreBarrio(uint8_t p, uint8_t c, uint8_t d, uint8_t b) {
#if DT_CON_BARRIOS
    const int i = IdxBarrio(p, c, d, b);
    return i < 0 ? std::string_view{} : Nombre(kBarrioNombre[i]);
#else
    (void)p; (void)c; (void)d; (void)b;
    return {};
#endif
}
 
std::string_view DivisionTerritorial::GetNombreBarrio(uint32_t codigoPostal) {
    uint8_t p, c, d, b;
    if (!DescomponerCodigoPostal(codigoPostal, p, c, d, b)) return {};
    return GetNombreBarrio(p, c, d, b);
}
 
std::string DivisionTerritorial::GetNombreCompleto(uint8_t p, uint8_t c, uint8_t d) {
    if (IdxDistrito(p, c, d) < 0) return {};
    std::string r;
    r.reserve(64);
    r.append(GetNombreProvincia(p));
    r.append(", ");
    r.append(GetNombreCanton(p, c));
    r.append(", ");
    r.append(GetNombreDistrito(p, c, d));
    return r;
}
 
uint32_t DivisionTerritorial::TotalProvincias() { return ContarValidos(kProvNombre, 0, kNumProv); }
 
uint32_t DivisionTerritorial::TotalCantones(uint8_t p) {
    return IdxProv(p) < 0 ? 0 : ContarValidos(kCantonNombre, kProvCantonIni[p - 1], kProvCantonIni[p]);
}
 
uint32_t DivisionTerritorial::TotalDistritos(uint8_t p, uint8_t c) {
    const int ci = IdxCanton(p, c);
    return ci < 0 ? 0 : ContarValidos(kDistNombre, kCantonDistIni[ci], kCantonDistIni[ci + 1]);
}
 
uint32_t DivisionTerritorial::TotalBarrios(uint8_t p, uint8_t c, uint8_t d) {
#if DT_CON_BARRIOS
    const int di = IdxDistrito(p, c, d);
    return di < 0 ? 0 : ContarValidos(kBarrioNombre, kDistBarrioIni[di], kDistBarrioIni[di + 1]);
#else
    (void)p; (void)c; (void)d;
    return 0;
#endif
}
 
bool DivisionTerritorial::ProvinciaPorIndice(uint32_t i, EntradaTerritorial& out) {
    const int k = NesimoValido(kProvNombre, 0, kNumProv, i);
    if (k < 0) return false;
    out = {uint8_t(k + 1), Nombre(kProvNombre[k])};
    return true;
}
 
bool DivisionTerritorial::CantonPorIndice(uint8_t p, uint32_t i, EntradaTerritorial& out) {
    if (IdxProv(p) < 0) return false;
    const unsigned ini = kProvCantonIni[p - 1];
    const int k = NesimoValido(kCantonNombre, ini, kProvCantonIni[p], i);
    if (k < 0) return false;
    out = {uint8_t(k - ini + 1), Nombre(kCantonNombre[k])};
    return true;
}
 
bool DivisionTerritorial::DistritoPorIndice(uint8_t p, uint8_t c, uint32_t i, EntradaTerritorial& out) {
    const int ci = IdxCanton(p, c);
    if (ci < 0) return false;
    const unsigned ini = kCantonDistIni[ci];
    const int k = NesimoValido(kDistNombre, ini, kCantonDistIni[ci + 1], i);
    if (k < 0) return false;
    out = {uint8_t(k - ini + 1), Nombre(kDistNombre[k])};
    return true;
}
 
bool DivisionTerritorial::BarrioPorIndice(uint8_t p, uint8_t c, uint8_t d, uint32_t i, EntradaTerritorial& out) {
#if DT_CON_BARRIOS
    const int di = IdxDistrito(p, c, d);
    if (di < 0) return false;
    const unsigned ini = kDistBarrioIni[di];
    const int k = NesimoValido(kBarrioNombre, ini, kDistBarrioIni[di + 1], i);
    if (k < 0) return false;
    out = {uint8_t(k - ini + 1), Nombre(kBarrioNombre[k])};
    return true;
#else
    (void)p; (void)c; (void)d; (void)i; (void)out;
    return false;
#endif
}
 
std::vector<EntradaTerritorial> DivisionTerritorial::ListarProvincias() {
    std::vector<EntradaTerritorial> v;
    v.reserve(kNumProv);
    for (unsigned i = 0; i < kNumProv; ++i)
        if (kProvNombre[i] != kNone) v.push_back({uint8_t(i + 1), Nombre(kProvNombre[i])});
    return v;
}
 
std::vector<EntradaTerritorial> DivisionTerritorial::ListarCantones(uint8_t p) {
    std::vector<EntradaTerritorial> v;
    if (IdxProv(p) < 0) return v;
    const unsigned ini = kProvCantonIni[p - 1], fin = kProvCantonIni[p];
    v.reserve(fin - ini);
    for (unsigned i = ini; i < fin; ++i)
        if (kCantonNombre[i] != kNone) v.push_back({uint8_t(i - ini + 1), Nombre(kCantonNombre[i])});
    return v;
}
 
std::vector<EntradaTerritorial> DivisionTerritorial::ListarDistritos(uint8_t p, uint8_t c) {
    std::vector<EntradaTerritorial> v;
    const int ci = IdxCanton(p, c);
    if (ci < 0) return v;
    const unsigned ini = kCantonDistIni[ci], fin = kCantonDistIni[ci + 1];
    v.reserve(fin - ini);
    for (unsigned i = ini; i < fin; ++i)
        if (kDistNombre[i] != kNone) v.push_back({uint8_t(i - ini + 1), Nombre(kDistNombre[i])});
    return v;
}
 
std::vector<EntradaTerritorial> DivisionTerritorial::ListarBarrios(uint8_t p, uint8_t c, uint8_t d) {
    std::vector<EntradaTerritorial> v;
#if DT_CON_BARRIOS
    const int di = IdxDistrito(p, c, d);
    if (di < 0) return v;
    const unsigned ini = kDistBarrioIni[di], fin = kDistBarrioIni[di + 1];
    v.reserve(fin - ini);
    for (unsigned i = ini; i < fin; ++i)
        if (kBarrioNombre[i] != kNone) v.push_back({uint8_t(i - ini + 1), Nombre(kBarrioNombre[i])});
#else
    (void)p; (void)c; (void)d;
#endif
    return v;
}

}  // namespace cec::common
