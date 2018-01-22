// events.cpp

#include <irrlicht.h>
#include <iostream>
#include "events.h"
#include "gui_ids.h"

using namespace irr;

namespace ic = irr::core;
namespace is = irr::scene;
namespace iv = irr::video;
namespace ig = irr::gui;

/**************************************************************************\
 * EventReceiver::EventReceiver                                           *
\**************************************************************************/
EventReceiver::EventReceiver()
    : gui(nullptr), node(nullptr), button_pressed(false), current_texture(0), menu_open(true), active_weapon(0)

{

    for (u32 i=0; i<irr::KEY_KEY_CODES_COUNT; ++i)
        KeyIsDown[i] = false;

}


bool EventReceiver::IsKeyDown(irr::EKEY_CODE keyCode) const
{
    return KeyIsDown[keyCode];
}




/*------------------------------------------------------------------------*\
 * EventReceiver::keyboard                                                *
\*------------------------------------------------------------------------*/
bool EventReceiver::keyboard(const SEvent &event)
{
    if (event.KeyInput.PressedDown)
    {
        switch (event.KeyInput.Key)
        {
        case KEY_ESCAPE:
            exit(0);

        case KEY_KEY_X: // Open/Close menu
            menu_open = !menu_open;
            break;
        case KEY_KEY_F: // Change weapon
            active_weapon += 1;
            if (active_weapon == 2)
                active_weapon = 0;
            break;
        default:;
        }
    }

    return false;
}

/*------------------------------------------------------------------------*\
 * EventReceiver::mouse                                                   *
\*------------------------------------------------------------------------*/
bool EventReceiver::mouse(const SEvent &event)
{
    switch(event.MouseInput.Event)
    {
    case EMIE_LMOUSE_PRESSED_DOWN:
        button_pressed = true;
        old_x = event.MouseInput.X;
        old_y = event.MouseInput.Y;
        break;
    case EMIE_LMOUSE_LEFT_UP:
        button_pressed = false;
        break;
    case EMIE_MOUSE_MOVED:
        if (true)
        {
            ic::vector3df rotation = node->getRotation();
            rotation.Y += 3*(event.MouseInput.X - old_x);
            old_x = event.MouseInput.X;
            old_y = event.MouseInput.Y;
            node->setRotation(rotation);
        }
        break;
    case EMIE_MOUSE_WHEEL:
        current_texture = (current_texture + 1) % textures.size();
        node->setMaterialTexture(0, textures[current_texture]);
        break;
    default:
        ;
    }

    return false;
}

bool EventReceiver::gui_handler(const SEvent &event)
{
    if (!node) return false;
    switch(event.GUIEvent.EventType)
    {
    // Menu bar management
    case ig::EGET_MENU_ITEM_SELECTED:
    {
        ig::IGUIContextMenu *menu = (ig::IGUIContextMenu*)event.GUIEvent.Caller;
        s32 item = menu->getSelectedItem();
        s32 id = menu->getItemCommandId(item);
        u32 debug_info = node->isDebugDataVisible();

        switch(id)
        {
        case MENU_NEW_GAME:
            break;
        case MENU_QUIT:
            exit(0);

        case MENU_BOUNDING_BOX:
            menu->setItemChecked(item, !menu->isItemChecked(item));
            node->setDebugDataVisible(debug_info ^ is::EDS_BBOX);
            break;

        case MENU_NORMALS:
            menu->setItemChecked(item, !menu->isItemChecked(item));
            node->setDebugDataVisible(debug_info ^ is::EDS_NORMALS);
            break;

        case MENU_TRIANGLES:
            menu->setItemChecked(item, !menu->isItemChecked(item));
            node->setDebugDataVisible(debug_info ^ is::EDS_MESH_WIRE_OVERLAY);
            break;

        case MENU_TRANSPARENCY:
            menu->setItemChecked(item, !menu->isItemChecked(item));
            node->setDebugDataVisible(debug_info ^ is::EDS_HALF_TRANSPARENCY);
            break;

        case MENU_ABOUT:
            gui->addMessageBox(L"About", L"In this apocalyptic world, survive to zombie squeletons and eliminate them all ! Press left click to shoot, X to pause, F to change weapon and ZQSD to move.");
            break;
        }
    }
        break;
        // management of text editing boxes
    case ig::EGET_EDITBOX_CHANGED:
    {
        s32 id = event.GUIEvent.Caller->getID();
        if (id == WINDOW_VALUE)
        {
            ic::stringc s = event.GUIEvent.Caller->getText();
            std::cout << "editbox changed:" << s.c_str() << std::endl;
        }
    }
        break;
        // buttons management
    case ig::EGET_BUTTON_CLICKED:
    {
        s32 id = event.GUIEvent.Caller->getID();
        if (id == WINDOW_BUTTON)
            std::cout << "Button clicked\n";
    }
        break;
        // check box management
    case ig::EGET_CHECKBOX_CHANGED:
    {
        s32 id = event.GUIEvent.Caller->getID();
        if (id == WINDOW_CHECK_BOX)
        {
            std::cout << "Check box clicked: ";
            bool checked = ((ig::IGUICheckBox*)event.GUIEvent.Caller)->isChecked();
            if (!checked) std::cout << "un";
            std::cout << "checked\n";
        }
    }
        break;
        // combo-box management
    case ig::EGET_COMBO_BOX_CHANGED:
    {
        s32 id = event.GUIEvent.Caller->getID();
        if (id == WINDOW_COMBO_BOX)
        {
            ig::IGUIComboBox *cbox = (ig::IGUIComboBox*)event.GUIEvent.Caller;
            s32 item = cbox->getSelected();
            u32 elem_id = cbox->getItemData(item);
            std::cout << "Combo box changed: item " << item << ", id " << elem_id << std::endl;
        }
    }
        break;
        // List management
    case ig::EGET_LISTBOX_CHANGED:
    {
        s32 id = event.GUIEvent.Caller->getID();
        if (id == WINDOW_LIST_BOX)
        {
            ig::IGUIListBox *lbox = (ig::IGUIListBox*)event.GUIEvent.Caller;
            s32 item = lbox->getSelected();
            std::cout << "List box changed: item " << item << std::endl;
        }
    }
        break;
        // Scroll bar management
    case ig::EGET_SCROLL_BAR_CHANGED:
    {
        s32 id = event.GUIEvent.Caller->getID();
        if (id == WINDOW_SCROLLBAR)
        {
            ig::IGUIScrollBar *scroll = (ig::IGUIScrollBar*)event.GUIEvent.Caller;
            s32 value = scroll->getPos();
            std::cout << "Scrollbar moved: " << value << std::endl;
        }
    }
        break;
        // Spin box management
    case ig::EGET_SPINBOX_CHANGED:
    {
        s32 id = event.GUIEvent.Caller->getID();
        if (id == WINDOW_SPIN_BOX)
        {
            ig::IGUISpinBox *spin = (ig::IGUISpinBox*)event.GUIEvent.Caller;
            f32 value = spin->getValue();
            std::cout << "Spin Box changed: " << value << std::endl;
        }
    }
        break;
    default:;
    }
    return false;
}

/**************************************************************************\
 * EventReceiver::OnEvent                                                 *
\**************************************************************************/
bool EventReceiver::OnEvent(const SEvent &event)
{
    if (!node) return false;
    switch (event.EventType)
    {
    case EET_KEY_INPUT_EVENT:
        KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
        return keyboard(event);
    case EET_MOUSE_INPUT_EVENT:
        return mouse(event);
    case EET_GUI_EVENT:
        return gui_handler(event);
    default:;
    }

    return false;
}

/**************************************************************************\
 * EventReceiver::is_mouse_pressed                                        *
\**************************************************************************/
bool EventReceiver::is_mouse_pressed(int &x, int &y)
{
    if (button_pressed)
    {
        x = old_x;
        y = old_y;
        return true;
    }
    return false;
}

/**************************************************************************\
 * EventReceiver::set_node                                                *
\**************************************************************************/
void EventReceiver::set_node(irr::scene::ISceneNode *n)
{
    node = n;
}

/**************************************************************************\
 * EventReceiver::set_gui                                                 *
\**************************************************************************/
void EventReceiver::set_gui(irr::gui::IGUIEnvironment *g)
{
    gui = g;
}
