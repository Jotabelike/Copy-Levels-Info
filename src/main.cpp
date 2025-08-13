#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/Mod.hpp>
#include <sstream>

using namespace geode::prelude;

static std::string getLevelDifficulty(GJGameLevel* level) {
    if (!level) return "Unknown";

    if (level->m_demon) {
        switch (level->m_demonDifficulty) {
        case 3: return "Easy Demon";
        case 4: return "Medium Demon";
        case 5: return "Insane Demon";
        case 6: return "Extreme Demon";
        default: return "Hard Demon";
        }
    }

    if (level->m_autoLevel || level->m_stars == 1) return "Auto";
    if (level->m_stars == 2) return "Easy";
    if (level->m_stars == 3) return "Normal";
    if (level->m_stars == 4 || level->m_stars == 5) return "Hard";
    if (level->m_stars == 6 || level->m_stars == 7) return "Harder";
    if (level->m_stars == 8 || level->m_stars == 9) return "Insane";
    if (level->m_stars == 0) return "Unrated";
    return "Unknown";
}

static std::string getLengthName(int levelLength) {
    switch (levelLength) {
    case 0: return "Tiny";
    case 1: return "Short";
    case 2: return "Medium";
    case 3: return "Long";
    case 4: return "XL";
    default: return "Unknown";
    }
}

static void copyLevelInfo(GJGameLevel* level) {
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

<<<<<<< HEAD
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
        info << "Difficulty: " << getLevelDifficulty(level) << "\n";
    if (copyObjects)
        info << "Objects: " << level->m_objectCount << "\n";
    if (copyDuration)
        info << "Duration: " << getLengthName(level->m_levelLength) << "\n";
=======
    bool includeLabels = true;
    // Si el setting a n no existe (usuarios antiguos), se mantiene en true
    try { includeLabels = mod->getSettingValue<bool>("copy-include-labels"); }
    catch (...) {}
>>>>>>> 7366d03 (update 1.2.5)

    std::stringstream info;

    auto appendLine = [&](bool cond, std::string const& label, std::string const& value) {
        if (!cond) return;
        if (includeLabels) info << label << ": " << value << "\n";
        else info << value << "\n";
        };

    appendLine(copyLevelID, "ID", std::to_string(level->m_levelID.value()));
    appendLine(copyLevelName, "Level Name", level->m_levelName.c_str());
    appendLine(copySongID, "Song ID", std::to_string(level->m_songID));
    appendLine(copyCreator, "Creator", level->m_creatorName.c_str());
    appendLine(copyStars, "Stars", std::to_string(level->m_stars));
    appendLine(copyDifficulty, "Difficulty", getLevelDifficulty(level));
    appendLine(copyObjects, "Objects (Est)", std::to_string(level->m_objectCount));
    appendLine(copyDuration, "Duration", getLengthName(level->m_levelLength));

    std::string out = info.str();
    if (!out.empty() && out.back() == '\n')
        out.pop_back();

    if (geode::utils::clipboard::write(out)) {
        Notification::create("Copied!", NotificationIcon::Success)->show();
    }
    else {
        Notification::create("Error", NotificationIcon::Error)->show();
    }
}

class $modify(LevelInfoLayerCopyHook, LevelInfoLayer) {
public:
    struct Fields {
        CCMenu* menu = nullptr;
        CCMenuItemSpriteExtra* button = nullptr;
    };

    bool init(GJGameLevel * level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        auto label = CCLabelBMFont::create("Copy", "bigFont.fnt");
        label->setScale(0.7f);
        auto spr = CircleButtonSprite::create(label, CircleBaseColor::Pink, CircleBaseSize::Medium);
        spr->setScale(0.65f);

        m_fields->button = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(LevelInfoLayerCopyHook::onCopy)
        );

        m_fields->menu = CCMenu::create();
        m_fields->menu->addChild(m_fields->button);

        if (auto likeBtn = this->getChildByID("like-button")) {
            m_fields->menu->setPosition(
                likeBtn->getPositionX() - 70.f,
                likeBtn->getPositionY()
            );
        }
        else {
            m_fields->menu->setPosition(106.f, 30.f);
        }

        if (auto bottom = this->getChildByID("bottom-menu"))
            bottom->addChild(m_fields->menu);
        else
            this->addChild(m_fields->menu);

        return true;
    }

    void onCopy(CCObject*) {
        copyLevelInfo(m_level);
    }
};
