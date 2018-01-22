    // main.cpp

    #include <iostream>

    #include <irrlicht.h>

    #include "events.h"
    #include "gui_ids.h"
    using namespace irr;

    namespace ic = irr::core;
    namespace is = irr::scene;
    namespace iv = irr::video;
    namespace ig = irr::gui;

    int shoot = 0; // shoot = 0 when hero is not shooting, 1 when he is shooting
    int active_camera = 0; // active_camera = 0 is hero camera, 1 is menu camera
    const int ENEMY_ID = 42; // ID of enemies to kill
    is::IAnimatedMeshSceneNode* node_to_remove; //change node when enemies die
    bool should_remove = false;
    bool running = false; // Hero is running when he moves (true)

    // Bounding boxes
    core::aabbox3d<f32> box1;
    core::aabbox3d<f32> box2;

    std::vector<core::aabbox3d<f32>> boxZombies;

    /*===========================================================================*\
    * create_menu                                                               *
    \*===========================================================================*/
    static void create_menu(ig::IGUIEnvironment *gui)
    {
	ig::IGUIContextMenu *submenu;

	// The three main entries:
	ig::IGUIContextMenu *menu = gui->addMenu();
	menu->addItem(L"File", -1, true, true);
	menu->addItem(L"Debug", -1, true, true);
	menu->addItem(L"Help", -1, true, true);

	// The contents of the File menu:
	submenu = menu->getSubMenu(0);
	submenu->addItem(L"New game...", MENU_NEW_GAME);
	submenu->addSeparator();
	submenu->addItem(L"Quit", MENU_QUIT);

	// The contents of the Debug menu:
	submenu = menu->getSubMenu(1);
	submenu->addItem(L"Bounding Box",   MENU_BOUNDING_BOX);
	submenu->addItem(L"Show Normals",   MENU_NORMALS);
	submenu->addItem(L"Show Triangles", MENU_TRIANGLES);
	submenu->addItem(L"Transparency",   MENU_TRANSPARENCY);

	// The contents of the Help menu:
	submenu = menu->getSubMenu(2);
    submenu->addItem(L"About...", MENU_ABOUT);
    }

    void scaleNode(scene::ISceneNode* mesh, float factor)
    {
	core::vector3df factorEscalate(factor,factor,factor);
	mesh->setScale(factorEscalate);
    }

    /*===========================================================================*\
    * create_window                                                             *
    \*===========================================================================*/
    static void create_window(ig::IGUIEnvironment *gui)
    {
	// The window
	ig::IGUIWindow *window = gui->addWindow(ic::rect<s32>(420,25, 620,460), false, L"Settings");

	// A text editing area, preceded by a label
	gui->addStaticText(L"Value", ic::rect<s32>(22,48, 65,66), false, false, window);
	gui->addEditBox(L"1.0", ic::rect<s32>(65,46, 160,66), true, window, WINDOW_VALUE);

	// A button to click
	gui->addButton(ic::rect<s32>(40,74, 140,92), window, WINDOW_BUTTON, L"Click me!");

	// A check box
	gui->addCheckBox(true, ic::rect<s32>(40,100, 140,118), window, WINDOW_CHECK_BOX, L"Select me!");

	// A combo box (a drop-down list)
	gui->addStaticText(L"Choose one: ", ic::rect<s32>(22,126, 100,142), false, false, window);
	ig::IGUIComboBox *cbox = gui->addComboBox(ic::rect<s32>(100,126, 180,142), window, WINDOW_COMBO_BOX);
	cbox->addItem(L"Choice 1", WINDOW_COMBO_CHOICE_1);
	cbox->addItem(L"Choice 2", WINDOW_COMBO_CHOICE_2);
	cbox->addItem(L"Choice 3", WINDOW_COMBO_CHOICE_3);

	// A drop-down list
	gui->addStaticText(L"List:", ic::rect<s32>(22,150, 65,168), false, false, window);
	ig::IGUIListBox *lbox = gui->addListBox(ic::rect<s32>(40,170, 160,242), window, WINDOW_LIST_BOX, true);
	lbox->addItem(L"First Entry");
	lbox->addItem(L"Second Entry");
	lbox->addItem(L"Third Entry");

	// A scroll bar
	gui->addScrollBar(true, ic::rect<s32>(22,250, 160,268), window, WINDOW_SCROLLBAR);

	// A spin box
	gui->addSpinBox(L"18.0", ic::rect<s32>(40,280, 160,298), true, window, WINDOW_SPIN_BOX);
    }

    // Detect when animation is over
    class test : public is::IAnimationEndCallBack
    {
    public:

	void OnAnimationEnd(is::IAnimatedMeshSceneNode* node)
	{
	    std::cout<<"End animation"<<std::endl;
	    node_to_remove = node;
	    should_remove = true;
	}
    };

    int main()
    {
	// The event manager
	EventReceiver receiver;
	iv::ITexture *base_texture;
	//iv::ITexture *wounded_texture;
	std::vector<iv::ITexture*> textures;

	// Creation of the window and the rendering system.
	IrrlichtDevice *device = createDevice(iv::EDT_OPENGL,
					      ic::dimension2d<u32>(640, 480),
					      16, false, false, false, &receiver);

	iv::IVideoDriver  *driver = device->getVideoDriver();
	is::ISceneManager *smgr = device->getSceneManager();
	ig::IGUIEnvironment *gui = device->getGUIEnvironment();

    base_texture    = driver->getTexture("data/base.pcx");

	// Add the archive which contains a complete level
	device->getFileSystem()->addFileArchive("data/akutadm1.pk3");


	//device->getCursorControl()->changeIcon( (ig::ECI_CROSS)selectedCursor, Context.Sprites[selectedSprite] );
	device->getCursorControl()->setActiveIcon( ig::ECI_CROSS );
	// }
	// We load a bsp (one level):
	is::IAnimatedMesh *mesh = smgr->getMesh("akutadm1.bsp");
	is::IMeshSceneNode *node;
	node = smgr->addOctreeSceneNode(mesh->getMesh(0), nullptr, -1, 1024);

	// Shifting so that our characters are in the background
	node->setPosition(core::vector3df(-1300,-104,-1249));

	// Load mesh
	is::IAnimatedMesh *mesh_zombies = smgr->getMesh("data/tris.md2");
	is::IAnimatedMesh *mesh_heros = smgr->getMesh("data/tris.md2");
	is::IAnimatedMesh *mesh_gun = smgr->getMesh("data/liz_patty_weapon.obj");
    is::IAnimatedMesh *mesh_gun_2 = smgr->getMesh("data/Shotgun.obj");


	// Load textures for numbers, lives and weapons
	iv::ITexture *digits[10];
	digits[0] = driver->getTexture("data/0.png");
	digits[1] = driver->getTexture("data/1.png");
	digits[2] = driver->getTexture("data/2.png");
	digits[3] = driver->getTexture("data/3.png");
	digits[4] = driver->getTexture("data/4.png");
	digits[5] = driver->getTexture("data/5.png");
	digits[6] = driver->getTexture("data/6.png");
	digits[7] = driver->getTexture("data/7.png");
	digits[8] = driver->getTexture("data/8.png");
	digits[9] = driver->getTexture("data/9.png");

	iv::ITexture *coeur = driver->getTexture("data/coeur.png");

	iv::ITexture *weapon[2];

	weapon[0] = driver->getTexture("data/pistol.png");
	weapon[1] = driver->getTexture("data/shotgun.png");

	iv::ITexture *menu_texture = driver->getTexture("data/menu.jpg");
    iv::ITexture *title_texture = driver->getTexture("data/title.png");
    iv::ITexture *pressX_texture = driver->getTexture("data/press_X.png");

	iv::ITexture *shoot_texture = driver->getTexture("data/shoot_texture.png");

	ig::IGUIImage *shoot_img = gui->addImage(ic::rect<s32>(310,160,  410,260)); shoot_img->setScaleImage(true);

	// Creating places for numbers
	ig::IGUIImage *score_10000 = gui->addImage(ic::rect<s32>(430,10,  470,50)); score_10000->setScaleImage(true);
	ig::IGUIImage *score_1000  = gui->addImage(ic::rect<s32>(470,10,  510,50)); score_1000->setScaleImage(true);
	ig::IGUIImage *score_100   = gui->addImage(ic::rect<s32>(510,10,  550,50)); score_100->setScaleImage(true);
	ig::IGUIImage *score_10    = gui->addImage(ic::rect<s32>(550,10, 590,50)); score_10->setScaleImage(true);
	ig::IGUIImage *score_1     = gui->addImage(ic::rect<s32>(590,10, 630,50)); score_1->setScaleImage(true);

	// Creating places for the hearts
	ig::IGUIImage *life_10000 = gui->addImage(ic::rect<s32>(10,10,  50,50)); life_10000->setScaleImage(true);
	ig::IGUIImage *life_1000  = gui->addImage(ic::rect<s32>(50,10,  90,50)); life_1000->setScaleImage(true);
	ig::IGUIImage *life_100   = gui->addImage(ic::rect<s32>(90,10,  130,50)); life_100->setScaleImage(true);
	ig::IGUIImage *life_10    = gui->addImage(ic::rect<s32>(130,10, 170,50)); life_10->setScaleImage(true);
	ig::IGUIImage *life_1     = gui->addImage(ic::rect<s32>(170,10, 210,50)); life_1->setScaleImage(true);

	// Creating places for weapons
	ig::IGUIImage *pistol = gui->addImage(ic::rect<s32>(20,430,  50,450)); pistol->setScaleImage(true);
	ig::IGUIImage *active_pistol = gui->addImage(ic::rect<s32>(10,410,  90,460)); active_pistol->setScaleImage(true);

	ig::IGUIImage *shotgun = gui->addImage(ic::rect<s32>(120,420,  180,450)); shotgun->setScaleImage(true);
	ig::IGUIImage *active_shotgun = gui->addImage(ic::rect<s32>(70,410,  200,470)); active_shotgun->setScaleImage(true);

    // Creating places for menu
	ig::IGUIImage *menu = gui->addImage(ic::rect<s32>(0,0,  650,500)); menu->setScaleImage(true);
    ig::IGUIImage *title = gui->addImage(ic::rect<s32>(225,150,  575,300));
    ig::IGUIImage *pressX = gui->addImage(ic::rect<s32>(225,350,  575,450));



	int score = 0; // Memorize  score which increase when an enemy dies
	textures.push_back(driver->getTexture("data/base.pcx"));
	textures.push_back(driver->getTexture("data/red_texture.pcx"));
	textures.push_back(driver->getTexture("data/blue_texture.pcx"));


    // Hero creation
	is::IAnimatedMeshSceneNode *heros = smgr->addAnimatedMeshSceneNode(mesh_heros);
	heros->setMaterialFlag(iv::EMF_LIGHTING, false);
	heros->setMD2Animation(is::EMAT_STAND);
	heros->setMaterialTexture(0, driver->getTexture("data/red_texture.pcx"));
	heros->setPosition(ic::vector3df(300,-90,-150));
	heros->setRotation(ic::vector3df(0,90,0));
	heros->setDebugDataVisible(is::EDS_BBOX);

	// Main weapon loading
	is::IAnimatedMeshSceneNode *gun = smgr->addAnimatedMeshSceneNode(mesh_gun);
	gun->setMaterialFlag(iv::EMF_LIGHTING, false);
	gun->setMaterialTexture(0, driver->getTexture("data/CW002_0001_T00.png"));
	gun->setPosition(heros->getPosition() + ic::vector3df(5,-2,-5));
	gun->setRotation(ic::vector3df(90,180,0));
	heros->addChild(gun);
	scaleNode(gun,0.4);

	// Secondary weapon loading
	is::IAnimatedMeshSceneNode *gun_2 = smgr->addAnimatedMeshSceneNode(mesh_gun_2);
    gun_2->setMaterialFlag(iv::EMF_LIGHTING, false);
	gun_2->setMaterialTexture(0, driver->getTexture("data/Sg_Diffuse.png"));
	gun_2->setPosition(heros->getPosition() + ic::vector3df(6,-1,-7));
	heros->addChild(gun_2);
	scaleNode(gun_2,2);


    // Enemies creation
	is::IAnimatedMeshSceneNode *node_zombies;
    is::IAnimatedMeshSceneNode *node_zombies2[6]; // tableau de zombie
	int alive_zombies[6] = {1,1,1,1,1,1};

	for (int i = -100; i <= 100; i += 40)
	{
	    node_zombies = smgr->addAnimatedMeshSceneNode(mesh_zombies);
	    node_zombies->setID(ENEMY_ID);
	    node_zombies->setMaterialFlag(iv::EMF_LIGHTING, false);
	    node_zombies->setMD2Animation(is::EMAT_RUN);
	    node_zombies->setMaterialTexture(0, base_texture);
	    node_zombies->setPosition(ic::vector3df(280+sqrt(300*300+150*150)*cos(i),-90, -130+sqrt(300*300+150*150)*sin(i)));
	    node_zombies->setRotation(ic::vector3df(0, -90, 0));
	    node_zombies->setDebugDataVisible(is::EDS_BBOX);


        is::ITriangleSelector *selector = smgr->createTriangleSelector(node_zombies); // create a triangle selector on each enemy to detect the collision with the shots
	    node_zombies->setTriangleSelector(selector);

	    // Triangle selector creation
	    scene::ITriangleSelector *selector1;
        selector1 = smgr->createOctreeTriangleSelector(node->getMesh(), node);      // creating a triangle selector to manage the collision with the ground
	    node->setTriangleSelector(selector1);
	    // And the animator / collision manager
	    scene::ISceneNodeAnimator *anim1;
	    anim1 = smgr->createCollisionResponseAnimator(selector1,
							node_zombies,  // The node we want to manage
							ic::vector3df(30, 30, 30), // "rays" from the camera
							ic::vector3df(0, -10, 0),  // gravity
							ic::vector3df(0, 0, 0));  // center shifting
	    node_zombies->addAnimator(anim1);
	    node_zombies2[(i+100)/40]=node_zombies;
	    anim1->drop();
	    selector1->drop();
	    selector->drop();
	}

	is::ISceneCollisionManager *collision_manager = smgr->getSceneCollisionManager();

	receiver.set_gui(gui);
	receiver.set_node(heros);
	receiver.set_textures(textures);


	//cameras
	is::ICameraSceneNode *camera[2]={0,0};
	camera[0] = smgr->addCameraSceneNode(heros, ic::vector3df(0, 0, -50), ic::vector3df(0, 5, 0));
	camera[1] = smgr->addCameraSceneNode(heros, ic::vector3df(-50, 20, 0), ic::vector3df(0, 0, 0));

	camera[0]->setPosition(heros->getPosition() + ic::vector3df(0, 50, 0));

	// Triangle selector creation
	scene::ITriangleSelector *selector;
	selector = smgr->createOctreeTriangleSelector(node->getMesh(), node);
	node->setTriangleSelector(selector);



    // And the animator / collision manager of the hero with the field
    scene::ISceneNodeAnimator *anim;
    anim = smgr->createCollisionResponseAnimator(selector,
						    heros,  // Le noeud que l'on veut gérer
						    ic::vector3df(30, 30, 30), // "rayons" de la caméra
						    ic::vector3df(0, -10, 0),  // gravité
						    ic::vector3df(0, 0, 0));  // décalage du centre
	heros->addAnimator(anim);





	// gui creation
	// Choice of the font
	ig::IGUISkin* skin = gui->getSkin();
	ig::IGUIFont* font = gui->getFont("data/fontlucida.png");
	skin->setFont(font);

	// The menu bar
	create_menu(gui);

	// A window for different settings
    //create_window(gui);

	life_10000->setImage(coeur);
	life_1000->setImage(coeur);
	life_100->setImage(coeur);
	life_10->setImage(coeur);
	life_1->setImage(coeur);

	pistol->setImage(weapon[0]);
	active_pistol->setImage(weapon[0]);

	shotgun->setImage(weapon[1]);
	active_shotgun->setImage(weapon[1]);

	menu->setImage(menu_texture);
    title->setImage(title_texture);
    pressX->setImage(pressX_texture);

	shoot_img->setImage(shoot_texture);

	test *testCallback = new test();
	//    u32 then = device->getTimer()->getTime();

	const f32 MOVEMENT_SPEED = 5.f; // Hero movement speed

	double life = 5; // Number of hero lives

    int sum = 0; // Number of alive enemies

    core::aabbox3d<f32> box_test0,box_test1,box_test2,box_test3,box_test4,box_test5;

	while(device->run())
	{
        u32 now = device->getTimer()->getTime();

        box1 = heros->getTransformedBoundingBox();


	    ic::vector3df position = heros->getPosition();
	    ic::vector3df rotation = heros->getRotation();


        if(!receiver.IsKeyDown(irr::KEY_KEY_Z) && !receiver.IsKeyDown(irr::KEY_KEY_Q) && !receiver.IsKeyDown(irr::KEY_KEY_D) && !receiver.IsKeyDown(irr::KEY_KEY_S))
	    {
		running = false;
		heros->setMD2Animation(is::EMAT_STAND);
	    }

	    if(receiver.IsKeyDown(irr::KEY_KEY_Z)) {
		if(running==false)
		{
		    heros->setMD2Animation(is::EMAT_RUN);
		}
		running = true;

        position.X += MOVEMENT_SPEED * cos(rotation.Y * M_PI / 180.0);
        position.Z += -MOVEMENT_SPEED * sin(rotation.Y * M_PI / 180.0);
	    }
        else if(receiver.IsKeyDown(irr::KEY_KEY_S)) {
        if(running==false)
        {
            heros->setMD2Animation(is::EMAT_RUN);
        }
        running = true;
        position.X += -0.5*MOVEMENT_SPEED * cos(rotation.Y * M_PI / 180.0);
        position.Z += 0.5*MOVEMENT_SPEED * sin(rotation.Y * M_PI / 180.0);
	    }
	    if(receiver.IsKeyDown(irr::KEY_KEY_Q))
	    {
		if(running==false)
		{
		    heros->setMD2Animation(is::EMAT_RUN);
		}
		running = true;
        position.X += 0.75*MOVEMENT_SPEED * cos(rotation.Y * M_PI  / 180.0 - M_PI/2) ;
        position.Z += -0.75*MOVEMENT_SPEED * sin(rotation.Y * M_PI / 180.0 - M_PI/2);
	    }

	    else if(receiver.IsKeyDown(irr::KEY_KEY_D))
	    {
		if(running==false)
		{
		    heros->setMD2Animation(is::EMAT_RUN);
		}
		running = true;
        position.X += 0.75*MOVEMENT_SPEED * cos(rotation.Y * M_PI  / 180.0 + M_PI/2) ;
        position.Z += -0.75*MOVEMENT_SPEED * sin(rotation.Y * M_PI / 180.0 + M_PI/2);
	    }
        heros->setPosition(position);

	    for (int i = 0; i <= 5; i ++){
          if (alive_zombies[i]==1 && !receiver.menu_open)
		{
		  node_zombies2[i]->setPosition(node_zombies2[i]->getPosition()+ ic::vector3df(( rand()/(double)RAND_MAX ) * 3 -1,0,( rand()/(double)RAND_MAX ) * 3 -1)+ (heros->getPosition()-node_zombies2[i]->getPosition())/heros->getPosition().getDistanceFrom(node_zombies2[i]->getPosition())*1.5);
		  ic::vector3df toTarget(heros->getPosition()-node_zombies2[i]->getPosition());
          node_zombies2[i]->setRotation(ic::vector3df(0,toTarget.Z+180,0));
          box_test0 = node_zombies2[i]->getTransformedBoundingBox();
          if(box1.intersectsWithBox(box_test0))
          {
              if(now > 2000)
              {
                life = life - 1;
                device->getTimer()->setTime(0);
              }
          }


		}
	    }
	    if(should_remove) {
		node_to_remove->remove();
		node_to_remove = nullptr;
		should_remove = false;
	    }

	    if (receiver.menu_open){
		active_camera = 0;
	    }
	    else{
		active_camera = 1;
	    }

	    smgr->setActiveCamera(camera[active_camera]);
	    camera[active_camera]->setTarget(heros->getPosition() + ic::vector3df(0,0,0));

	    driver->beginScene(true, true, iv::SColor(100,150,100,255));

	    int mouse_x, mouse_y;
	    if (receiver.is_mouse_pressed(mouse_x, mouse_y))
	    {

		ic::line3d<f32> ray;
		ray = collision_manager->getRayFromScreenCoordinates(ic::position2d<s32>(mouse_x, mouse_y));
		ic::vector3df intersection;
		ic::triangle3df hit_triangle;

        // When hero is shooting, detect if he hits an enemy
		shoot = 1;
		is::ISceneNode *selected_scene_node =
			collision_manager->getSceneNodeAndCollisionPointFromRay(
			    ray,
			    intersection, // Here we retrieve the 3D coordinates of the intersection
			    hit_triangle, // and the intersected triangle
			    ENEMY_ID); // We only want nodes with this identifier

        // If an enemy got hit, he disappear and score increase
		if (selected_scene_node && selected_scene_node->getID() == ENEMY_ID) {
		    score += 500;
		    is::IAnimatedMeshSceneNode* selectedNode = static_cast<is::IAnimatedMeshSceneNode*>(selected_scene_node);
		    for (int i = 0; i <= 5; i ++){
		    if (selectedNode==node_zombies2[i])
			alive_zombies[i]=0;
		    }
		    selectedNode->setAnimationEndCallback(testCallback);
		    selectedNode->setMD2Animation(is::EMAT_CROUCH_DEATH );
		    selectedNode->setLoopMode(false);
		}
	    }


	    // Score update :
	    score_10000->setImage(digits[(score / 10000) % 10]);
	    score_1000->setImage(digits[(score / 1000) % 10]);
	    score_100->setImage(digits[(score / 100) % 10]);
	    score_10->setImage(digits[(score / 10) % 10]);
	    score_1->setImage(digits[(score / 1) % 10]);

	    // Scene drawing :
	    smgr->drawAll();

	    // Score display :
	    score_10000->draw();
	    score_1000->draw();
	    score_100->draw();
	    score_10->draw();
	    score_1->draw();


	    // Lives display :
	    if(life>=1)
		life_10000->draw();
	    if(life>=2)
		life_1000->draw();
	    if(life>=3)
		life_100->draw();
	    if(life>=4)
		life_10->draw();
	    if(life>=5)
		life_1->draw();

        // Calculate the number of alive enemies
        for(int i = 0 ; i<6 ; i++)
        {
            sum = sum + alive_zombies[i];
        }

        // If enemies are all dead, hero wins the game
        if(sum == 0)
        {
            wchar_t message[100];
            swprintf(message, 100, L"Win", 300);
            is::IBillboardTextSceneNode *end_game = smgr->addBillboardTextSceneNode(font, message, heros ,ic::dimension2d<f32>(50, 50));
        }
        sum = 0;

        // If hero looses all his lives, hero looses the game
	    if(life==0)
	    {
		wchar_t message[100];
		swprintf(message, 100, L"Game over", 300);
		is::IBillboardTextSceneNode *end_game = smgr->addBillboardTextSceneNode(font, message, heros ,ic::dimension2d<f32>(50, 50));
	    }

	    // Weapon display :

	    if (receiver.active_weapon == 0){
		active_pistol->draw();
		gun->setPosition(ic::vector3df(5,-2,-5));
		gun->setRotation(ic::vector3df(90,180,0));
		gun_2->setPosition(ic::vector3df(-3,10,-2));
		gun_2->setRotation(ic::vector3df(90,-30,-90));
		scaleNode(gun,0.4);
		scaleNode(gun_2,1.5);
	    }
	    else
		pistol->draw();

	    if (receiver.active_weapon == 1){
		active_shotgun->draw();
		gun->setPosition(ic::vector3df(5,5,0));
		gun->setRotation(ic::vector3df(0,0,90));
		gun_2->setPosition(ic::vector3df(6,-1,-7));
		gun_2->setRotation(ic::vector3df(0,0,0));
		scaleNode(gun,0.3);
		scaleNode(gun_2,2);
	    }
	    else
		shotgun->draw();

	    if (shoot==1){
		shoot_img->draw();
		shoot=0;
		    }

	    // Draw user interface :
	    if (receiver.menu_open){
		gui->drawAll(); }

	    driver->endScene();
	}
	device->drop();

	return 0;
    }
