#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class GoalPopup : public geode::Popup {
protected:
    TextInput* m_goalInput = nullptr;
    int m_levelID = 0;

    bool init(int levelID) {
        if (!geode::Popup::init(280.f, 180.f)) {
            return false;
        }

        m_levelID = levelID;

        this->setTitle("Set Goal");

        auto winSize = CCDirector::get()->getWinSize();

        auto label = CCLabelBMFont::create(
            "Goal percentage",
            "bigFont.fnt"
        );
        label->setScale(0.4f);
        label->setPosition({
            winSize.width / 2.f,
            winSize.height / 2.f + 40.f
        });
        m_mainLayer->addChild(label);

        m_goalInput = TextInput::create(
            160.f,
            "Enter 1-100",
            "bigFont.fnt"
        );
        m_goalInput->setScale(0.8f);
        m_goalInput->setPosition({
            winSize.width / 2.f,
            winSize.height / 2.f
        });
        m_mainLayer->addChild(m_goalInput);

        auto savedGoal = Mod::get()->getSavedValue<int>(
            fmt::format("goal-{}", m_levelID),
            0
        );

        if (savedGoal > 0) {
            m_goalInput->setString(std::to_string(savedGoal));
        }

        auto saveSprite = ButtonSprite::create("Save");
        saveSprite->setScale(0.5f);

        auto saveButton = CCMenuItemSpriteExtra::create(
            saveSprite,
            nullptr,
            this,
            menu_selector(GoalPopup::onSave)
        );
        saveButton->setPosition({
            winSize.width / 2.f,
            winSize.height / 2.f - 50.f
        });

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->addChild(saveButton);
        m_mainLayer->addChild(menu);

        return true;
    }

    void onSave(CCObject*) {
        auto text = m_goalInput->getString();
        int goal = 0;

        try {
            goal = std::stoi(text);
        } catch (...) {
            goal = 0;
        }

        if (goal < 1 || goal > 100) {
            FLAlertLayer::create(
                "Session Goals",
                "Enter a percentage between 1 and 100.",
                "OK"
            )->show();
            return;
        }

        Mod::get()->setSavedValue<int>(
            fmt::format("goal-{}", m_levelID),
            goal
        );

        this->onClose(nullptr);
    }

public:
    static GoalPopup* create(int levelID) {
        auto result = new GoalPopup();

        if (result && result->init(levelID)) {
            result->autorelease();
            return result;
        }

        CC_SAFE_DELETE(result);
        return nullptr;
    }
};

class $modify(GoalsPlayLayer, PlayLayer) {
    CCLabelBMFont* m_goalHUD = nullptr;
    bool m_goalReached = false;

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        m_goalReached = false;

        auto winSize = CCDirector::get()->getWinSize();

        m_goalHUD = CCLabelBMFont::create("", "bigFont.fnt");
        m_goalHUD->setScale(0.35f);
        m_goalHUD->setAnchorPoint({ 0.f, 1.f });
        m_goalHUD->setPosition({
            10.f,
            winSize.height - 60.f
        });

        this->addChild(m_goalHUD, 1000);

        this->updateHUD();

        return true;
    }

    void updateHUD() {
        if (!m_goalHUD || !m_level) {
            return;
        }

        auto levelID = m_level->m_levelID.value();
        auto goal = Mod::get()->getSavedValue<int>(
            fmt::format("goal-{}", levelID),
            0
        );

        auto best = m_level->m_normalPercent.value();
        auto attempts = m_level->m_attempts.value();

        if (goal <= 0) {
            m_goalHUD->setString(
                fmt::format(
                    "Best: {}%
Attempts: {}",
                    best,
                    attempts
                ).c_str()
            );
            return;
        }

        m_goalHUD->setString(
            fmt::format(
                "Goal: {}%
Best: {}%
Attempts: {}",
                goal,
                best,
                attempts
            ).c_str()
        );

        if (!m_goalReached && best >= goal) {
            m_goalReached = true;

            FLAlertLayer::create(
                "Session Goals",
                fmt::format(
                    "Goal complete!
You reached {}%.",
                    best
                ).c_str(),
                "OK"
            )->show();

            FMODAudioEngine::get()->playEffect(
                "achievement_01.ogg"
            );
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        this->updateHUD();
    }
};

class $modify(GoalsPauseLayer, PauseLayer) {
    bool init(GJGameLevel* level) {
        if (!PauseLayer::init(level)) {
            return false;
        }

        auto sprite = ButtonSprite::create("Goal");
        sprite->setScale(0.5f);

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            nullptr,
            this,
            menu_selector(GoalsPauseLayer::onSetGoal)
        );

        auto winSize = CCDirector::get()->getWinSize();

        button->setPosition({
            winSize.width - 70.f,
            winSize.height - 70.f
        });

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->addChild(button);

        this->addChild(menu, 1000);

        return true;
    }

    void onSetGoal(CCObject*) {
        auto playLayer = PlayLayer::get();

        if (!playLayer || !playLayer->m_level) {
            return;
        }

        auto popup = GoalPopup::create(
            playLayer->m_level->m_levelID.value()
        );

        if (popup) {
            popup->show();
        }
    }
};
