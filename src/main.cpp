#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/Mod.hpp>
#include <sstream>

using namespace geode::prelude;

std::string getLevelDifficulty(GJGameLevel* level) {
    if (!level) return "Unknown";

    if (level->m_demon) {
        switch (level->m_demonDifficulty) {
        case 3: return "Easy Demon";
        case 4: return "Medium Demon";
        case 5: return "Insane Demon";
        case 6: return "Extreme Demon";
        }
        return "Hard Demon";
    }

    if (level->m_autoLevel || level->m_stars == 1)
        return "Auto";
    if (level->m_stars == 2)
        return "Easy";
    if (level->m_stars == 3)
        return "Normal";
    if (level->m_stars == 4 || level->m_stars == 5)
        return "Hard";
    if (level->m_stars == 6 || level->m_stars == 7)
        return "Harder";
    if (level->m_stars == 8 || level->m_stars == 9)
        return "Insane";
    if (level->m_stars == 0)
        return "Unrated";

    return "Unknown";
}

// Devuelve la duración como texto legible
std::string getLengthName(int levelLength) {
    switch (levelLength) {
    case 0: return "Tiny";
    case 1: return "Short";
    case 2: return "Medium";
    case 3: return "Long";
    case 4: return "XL";
    default: return "Unknown";
    }
}

void copyLevelInfo(GJGameLevel* level) {
    if (!level) {
        Notification::create("Error: invalid level", NotificationIcon::Error)->show();
        return;
    }

    auto mod = Mod::get();

    bool copyLevelID = mod->getSettingValue<bool>("copy-level-id");
    bool copyLevelName = mod->getSettingValue<bool>("copy-level-name");
    bool copySongID = mod->getSettingValue<bool>("copy-song-id");
    bool copyCreator = mod->getSettingValue<bool>("copy-creator");
    bool copyStars = mod->getSettingValue<bool>("copy-stars");
    bool copyDifficulty = mod->getSettingValue<bool>("copy-difficulty");
    bool copyObjects = mod->getSettingValue<bool>("copy-objects");
    bool copyDuration = mod->getSettingValue<bool>("copy-duration");

    std::stringstream info;
    if (copyLevelID)
        info << "ID: " << level->m_levelID.value() << "\n";
    if (copyLevelName)
        info << "Level Name: " << level->m_levelName.c_str() << "\n";
    if (copySongID)
        info << "Song ID: " << level->m_songID << "\n";
    if (copyCreator)
        info << "Creator: " << level->m_creatorName.c_str() << "\n";
    if (copyStars)
        info << "Stars: " << level->m_stars << "\n";
    if (copyDifficulty)
        info << "Dificulty: " << getLevelDifficulty(level) << "\n";
    if (copyObjects)
        info << "Objects: " << level->m_objectCount << "\n";
    if (copyDuration)
        info << "Duration: " << getLengthName(level->m_levelLength) << "\n";

    if (geode::utils::clipboard::write(info.str())) {
        Notification::create("Copied!", NotificationIcon::Success)->show();
    }
    else {
        Notification::create("Error", NotificationIcon::Error)->show();
    }
}

class $modify(LevelInfoLayerCopyHook, LevelInfoLayer) {
public:
    struct Fields {
        CCMenu* m_copyMenu = nullptr;
        CCMenuItemSpriteExtra* m_copyBtn = nullptr;
    };

    bool init(GJGameLevel * level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        auto label = CCLabelBMFont::create("COPY", "bigFont.fnt");
        label->setScale(0.7f);
        auto copySpr = CircleButtonSprite::create(label, CircleBaseColor::Pink, CircleBaseSize::Medium);
        copySpr->setScale(0.65f);

        m_fields->m_copyBtn = CCMenuItemSpriteExtra::create(
            copySpr,
            this,
            menu_selector(LevelInfoLayerCopyHook::onCopy)
        );

        m_fields->m_copyMenu = CCMenu::create();
        m_fields->m_copyMenu->addChild(m_fields->m_copyBtn);

        if (auto likeBtn = this->getChildByID("like-button")) {
            m_fields->m_copyMenu->setPosition(
                likeBtn->getPositionX() - 70.f,
                likeBtn->getPositionY()
            );
        }
        else {
            m_fields->m_copyMenu->setPosition(106.f, 30.f);
        }

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(m_fields->m_copyMenu);
        }
        else {
            this->addChild(m_fields->m_copyMenu);
        }

        return true;
    }

    void onCopy(CCObject*) {
        copyLevelInfo(m_level);
    }
};