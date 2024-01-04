**TP Linux Embarqué**

**1.** **Prise en main :**\
**1.1.** **Préparation de la carte SD**\
Premièrement, nous avons procédé au téléchargement des ressources
requises pour cette séance pratique. Ensuite, nous avons utilisé le
logiciel Win32DiskImager sous Windows pour

<table style="width:100%;">
<colgroup>
<col style="width: 10%" />
<col style="width: 10%" />
<col style="width: 10%" />
<col style="width: 10%" />
<col style="width: 10%" />
<col style="width: 10%" />
<col style="width: 10%" />
<col style="width: 10%" />
<col style="width: 10%" />
<col style="width: 10%" />
</colgroup>
<thead>
<tr class="header">
<th>flasher</th>
<th>la</th>
<th>carte</th>
<th>SD</th>
<th>avec</th>
<th>l'image</th>
<th>fournie</th>
<th>par</th>
<th>Terasic,</th>
<th><blockquote>
<p>nommée</p>
</blockquote></th>
</tr>
</thead>
<tbody>
</tbody>
</table>

VEEK_MT2S_LXDE/VEEK_MT2S_LXDE.img.

**1.2.** **Démarrage**\
Nous avons introduit la carte SD programmée et mis en marche la carte.
Sur le bureau de LXDE, nous avons accès aux commandes pour les LEDs, les
afficheurs 7 segments, les boutons, les interrupteurs, le gyro et
l\'écran LCD. De plus, nous disposons d\'un navigateur et de toutes les
applications Linux disponibles.

**1.3.** **Connexion au système**\
**1.3.1.Liaison série**\
Nous avons employé le câble USB mini : UART vers USB avec les paramètres
suivants :

• Identifiant : root

• Mot de passe : aucun (appuyez simplement sur Entrée)\
Ensuite, nous avons redémarré le SoC afin d\'observer la séquence de
démarrage en utilisant la commande reboot. Pendant ce redémarrage, tous
les processus sont interrompus. Le système propose une option
d\'annulation du redémarrage, puis il lance le noyau.

> ![](media/image1.png){width="6.205555555555556in"height="3.322221128608924in"}

**1.3.2.Configuration réseau**\
Lors de la connexion de la carte VEEK sur le switch, l\'adresse qui nous
est assignée À l'aide de la commande **ifconfig** est la suivante :
**192.168.88.73.**

Après avoir rebooté le système une nouvelle adresse nous es attribué,
cette dernière est:

**192.168.88.68** Donc l'adresse peut être modifiée lors d'un reboot, le
ping fonctionne correctement.

.

![](media/image2.png){width="6.8875in"
height="4.3694444444444445in"}

> ![](media/image3.png){width="6.001388888888889in"
> height="2.5069444444444446in"}

**1.4.** **Découverte de la cible**\
**1.4.1.Exploration des dossiers /sys/class et /proc**

Sous la racine, nous retrouvons les répertoires suivants **: /usr,
/proc, /root, /sys,** etc. En exécutant la commande **cpuinfo**, on
constate la présence d\'un processeur dual core. Le fichier ioports est
vide, car sur un processeur ARM, les périphériques sont mappés dans la
mémoire (ce qui nécessite l\'utilisation de **iomem**). En revanche, sur
un PC ou une machine virtuelle, les périphériques ne partagent pas le
même bus, d\'où la nécessité de passer par **le ioports** pour y
accéder. Le répertoire /sys/class renferme également des périphériques
supplémentaires tels que GPIO, DMA, I2C, etc.

**1.4.2.Hello world !**

Lorsque l\'on tente d\'exécuter le fichier sur la machine virtuelle,
cela s\'avère impossible en

raison de sa nature binaire. Cependant, son exécution sur la carte
fonctionne parfaitement.

**1.4.3.** **Accès au matériel**

Un certain nombre de drivers sont fournis. Comme tous les drivers sous
Linux,

ils sont accessible sous forme de fichiers. Par exemple pour allumer
l'une des LED

rouge de la carte, il suffit d'écrire un '1' dans le bon fichier.

**echo \"1\" \> /sys/class/leds/fpga_led1/brightness**

![](media/image4.png){width="6.336111111111111in"
height="3.8652777777777776in"}

**1.4.4.** **Chenillard (Et oui, encore !)**\
Nous avons développé le code pour créer un effet de chenillard, et il
fonctionne de manière optimale.

![](media/image5.png){width="6.2972222222222225in"
height="2.8958333333333335in"}

![](media/image6.png){width="6.3in"
height="4.041666666666667in"}

> Dans ce code, nous accédons aux fichiers fpga_led1/brightness,
> fpga_led2/brightness, etc., puis nous modifions les valeurs des LEDs
> en effectuant une écriture.
>
> **2. Modules kernel (TP2)**
>
> **2.1.** **Reprise du TP1**

Nous avons établi une connexion réussie et une communication avec la
carte VEEK. Toutefois, nous avons dû ajuster l\'adresse IP lors de la
connexion à Putty, car celle-ci avait changé.

**2.2.Accès aux registres**\
Un programme qui accède directement aux registres depuis l'espace
utilisateur.

> ![](media/image7.png){width="6.3in"
> height="4.293055555555555in"}
>
> **Limitations de mmap():**
>
> Les mappages de mémoire sont toujours ajustés à des multiples entiers
> de pages, entraînant un gaspillage d\'espace libre. Avec des pages de
> 4 Ko, par exemple, un mappage de 7 octets gaspille 4 089 octets,
> surtout pour les petits fichiers. Ils doivent s\'insérer dans
> l\'espace d\'adressage du processus. En environnement 32 bits, de
> nombreux mappages de tailles différentes peuvent fragmenter l\'espace
> d\'adressage, compliquant la recherche de régions libres continues.
> Cette problématique est bien moins prononcée en 64 bits. La création
> et la gestion des mappages de mémoire engendrent une surcharge,
> généralement évitée en éliminant la double copie, surtout pour les
> fichiers volumineux et fréquemment utilisés.
>
> **2.3.** **Compilation de module noyau sur la VM**
>
> Pour compiler des modules noyau dans la VM, nous avons besoin des
> paquets suivant :
>
> ![](media/image8.png){width="6.294444444444444in"
> height="0.6624989063867016in"}

En utilisant le Makefile et le fichier source hello.c disponibles sur
Moodle, nous avons réussi à compiler notre premier module.

Les commandes suivantes ont été utilisées :\
- Pour charger le module : \`sudo insmod \<nom_du_module.ko\>\`\
- Pour décharger le module : \`sudo rmmod \<nom_du_module.ko\>\`\
- Pour obtenir des informations sur le module : \`sudo modinfo
\<nom_du_module.ko\>\` - \`sudo lsmod\` : pour afficher le statut des
modules dans le noyau Linux, fournissant une liste des modules chargés.

\- \`sudo dmesg\` : permet d\'afficher les messages dans le journal du
noyau.

Nous avons testé l\'ensemble de ces fonctions, et elles ont toutes
fonctionné de manière optimale.

Pour ajouter un paramètre, nous avons ajouté quelques lignes:

> ![](media/image9.png){width="4.963888888888889in"
> height="1.9291666666666667in"}

Pour inclure une entrée dans le système de fichiers proc :

> ![](media/image10.png){width="4.858332239720035in"
> height="4.1888877952755905in"}

Pour ajouter un timer:

> ![](media/image11.png){width="4.1875in"
> height="1.7777777777777777in"}
>
> **2.4.** **Récupération du Noyau Terasic**
>
> c'est déjà fait dans la VM !

**2.4.1.** **Préparation de la compilation**

> ![](media/image12.png){width="4.683333333333334in"
> height="0.6347222222222222in"}

Il faut bien retenir ce chemin car on en aura besoin pour la suite.

> ![](media/image13.png){width="5.854166666666667in"
> height="4.344444444444444in"}

**2.4.2.** **Récupération de la configuration actuelle du noyau**

> ***--- Quel est le rôle des lignes commençant par export ?***

Elles sont destinées à établir des variables d\'environnement pouvant
être exploitées

> tout au long de l\'exécution du shell.
>
> ***--- Pourquoi le chemin fini par un tiret \"-\" ?***
>
> Cela s\'explique par le fait que la ligne dans le makefile est suivie
> d\'une commande gcc,
>
> éliminant ainsi la nécessité de le spécifier explicitement.

+-----------------------------------+-----------------------------------+
| **2.4.3.**                        | > **Hello World**\                |
|                                   | > La compilation se déroule sans  |
| **2.4.4.**                        | > problème. En chargeant le       |
|                                   | > module et en utilisant la       |
|                                   | >                                 |
|                                   | > commande dmesg, on peut         |
|                                   | > observer l\'affichage du        |
|                                   | > message \"Hello, World\".       |
|                                   | >                                 |
|                                   | > **Chenillard (Yes !)**          |
+===================================+===================================+
+-----------------------------------+-----------------------------------+

> Au sein de cette section, nous avons utilisé un timer pour ajuster la
> fréquence d\'affichage en fonction d\'un paramètre d\'entrée. Les
> résultats sont probants : en modifiant ce paramètre, la vitesse
> d\'affichage dans dmesg fluctue. Pour confirmer cela, nous avons
> effectué des tests en configurant une vitesse très rapide, où chaque
> appel à dmesg révèle des changements fréquents, puis une vitesse très
> lente, où chaque appel à dmesg expose un changement à chaque fois.
>
> Ce clignotement prend en compte le motif défini dans un processus
> (proc). Dans notre cas, ce motif agit en altérant la direction de
> l\'affichage. Plutôt que de suivre
>
> une séquence ascendante telle que \"led1 led2 led3\", l\'affichage
> adopte une
>
> séquence décroissante, présentant \"led3 led2 led1\".
>
> **3.Device tree (TP3)**
>
> ![](media/image14.png){width="5.4944444444444445in"
> height="3.275in"}
>
> **3.1.** **module accedant au LED via /dev**
>
> ***Quel sont les rôles des principales fonctions (probe, read, write,
> remove), et***
>
> ***quand entrent-elles en action ?***
>
> La fonction **probe** est invoquée lorsque le noyau détecte le
> périphérique des LEDs. Elle est appelée une seule fois et a pour rôle
> de réserver la mémoire, créer la structure pour stocker les
> informations telles que les registres, allumer les LEDs et initialiser
> le misc device.
>
> La fonction **read**, quand à elle, est utilisée pour lire la valeur
> des LEDs au moment de son appel, tandis que la fonction **write**
> permet de modifier la valeur des LEDs.
>
> Enfin, la fonction **remove** est dédiée à la suppression du pilote
> lorsque le périphérique n\'existe plus.

![](media/image15.png){width="6.3in"
height="1.9861111111111112in"}

> On a ajouté des fonctions read et write pour chaque proc.
>
> La commande cat attend un caractère de fin de fichier.
>
> On remarque que la valeur est affichée en boucle lorsqu'on fait appel
> à cat, pour remédier à ce problème on a du créer une condition qui
> retourne un 0 si elle est respecté (0 pour fin de fichier), on a de
> nouveau tester notre programme avec le bout de code ajouté et nous
> avons pu constater le bon fonctionnement du command cat.

![](media/image16.png){width="6.3in"
height="3.3430555555555554in"}

> le résultat de ce changement est ci-dessous:

![](media/image17.png){width="6.3in"
height="3.6972222222222224in"}

> Par la suite nous avons déclaré une variable globale pour récupérer
> la structure qui s'occupe des leds et l'utiliser dans le
> timer_callback: pour changer la valeur des LEDs :
>
> ![](media/image18.png){width="6.3in" height="0.3527777777777778in"}

> les deux lignes ci-dessus permettent de modifier la valeur des LEDS ,
> ainsi en allant lire les fichiers:\
> -/proc/ensea/speed\
> -/dev/ensea-led\
> -/proc/ensea/dir\
> On récupère la valeur des variables puis par la suite on agit sur la
> vitesse, la direction et le pattern.
