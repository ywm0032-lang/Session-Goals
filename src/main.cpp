#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

class GoalPopup : public FLAlertLayer, public FLAlertLayerProtocol {
protected:
    TextInput* m_levelIdInput;
    TextInput* m_goalInput;

    bool init() {
        if (!FLAlertLayer::init(
            this,
            "Set Goal",
            "",
            "Cancel",
            "",
            320.f,
            false,
            230.f,
            1.f
        )) {
            return false;
        }

        this->setBrownBackground();

        float centerX = 160.f;

        auto levelLabel = CCLabelBMFont::create("Level ID", "bigFont.fnt");
        levelLabel->setScale(0.5f);
        levelLabel->setPosition({centerX, 175.f});
        m_mainLayer->addChild(levelLabel);

        m_levelIdInput = TextInput::create(220.f, "Level ID");
        m_levelIdInput->setPosition({centerX, 145.f});
        m_mainLayer->addChild(m_levelIdInput);

        auto goalLabel = CCLabelBMFont::create("Goal Percentage", "bigFont.fnt");
        goalLabel->setScale(0.5f);
        goalLabel->setPosition({centerX, 100.f});
        m_mainLayer->addChild(goalLabel);

        m_goalInput = TextInput::create(220.f, "Goal %");
        m_goalInput->setPosition({centerX, 70.f});
        m_mainLayer->addChild(m_goalInput);

        auto saveButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Save"),
            this,
            menu_selector(GoalPopup::onSave)
        );
        saveButton->setPosition({centerX, 25.f});

        auto menu = CCMenu::create();
        menu->addChild(saveButton);
        menu->setPosition({0.f, 0.f});
        m_buttonMenu->addChild(menu);

        return true;
    }

    void setBrownBackground() {
        for (int i = 0; i < m_mainLayer->getChildrenCount(); i++) {
            auto child = typeinfo_pointer_cast<CCScale9Sprite>(
                m_mainLayer->getChildren()->objectAtIndex(i)
            );

            if (child) {
                child->setSpriteFrame("GJ_square04.png"_spr);
                break;
            }
        }
    }

    void onSave(CCObject*) {
        std::string levelId = m_levelIdInput->getString();
        std::string goalText = m_goalInput->getString();

        if (levelId.empty() || goalText.empty()) {
            Notification::create(
                "Enter a level ID and goal.",
                NotificationIcon::Warning,
                2.f
            )->show();
            return;
        }

        int goal = std::atoi(goalText.c_str());

        if (goal < 1 || goal > 100) {
            Notification::create(
                "Goal must be between 1% and 100%.",
                NotificationIcon::Warning,
                2.f
            )->show();
            return;
        }

        Mod::get()->setSavedValue("goal-level", levelId);
        Mod::get()->setSavedValue("goal-percent", goal);
        Mod::get()->setSavedValue("goal-attempts", 0);

        Notification::create(
            "Goal saved.",
            NotificationIcon::Success,
            2.f
        )->show();

        this->onBtn1(nullptr);
    }

public:
    static GoalPopup* create() {
        auto result = new GoalPopup();

        if (result->init()) {
            result->autorelease();
            return result;
        }

        delete result;
        return nullptr;
    }
};

class GoalsPlayLayer : public Modify<GoalsPlayLayer, PlayLayer> {
public:
    struct Fields {
        CCLabelBMFont* m_goalLabel = nullptr;
    };

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);

        int attempts = Mod::get()->getSavedValue<int>("goal-attempts", 0);
        Mod::get()->setSavedValue("goal-attempts", attempts + 1);

        this->updateGoalLabel();
    }

    void updateGoalLabel() {
        std::string levelId = Mod::get()->getSavedValue<std::string>("goal-level", "");
        int goal = Mod::get()->getSavedValue<int>("goal-percent", 0);
        int attempts = Mod::get()->getSavedValue<int>("goal-attempts", 0);

        if (levelId.empty() || goal <= 0) {
            return;
        }

        if (!m_fields->m_goalLabel) {
            m_fields->m_goalLabel = CCLabelBMFont::create("", "bigFont.fnt");
            m_fields->m_goalLabel->setAnchorPoint({0.5f, 1.f});
            m_fields->m_goalLabel->setScale(0.4f);
            m_fields->m_goalLabel->setZOrder(999);
            m_fields->m_goalLabel->setID("session-goals-label"_spr);

            auto winSize = CCDirector::get()->getWinSize();
            m_fields->m_goalLabel->setPosition({
                winSize.width / 2.f,
                winSize.height - 10.f
            });

            m_uiLayer->addChild(m_fields->m_goalLabel);
        }

        std::string text = fmt::format(
            "Level: {}  Goal: {}%  Attempts: {}",
            levelId,
            goal,
            attempts
        );

        m_fields->m_goalLabel->setString(text.c_str());
    }
};

class GoalsPauseLayer : public Modify<GoalsPauseLayer, PauseLayer> {
public:
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::get()->getWinSize();

        auto goalsButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Goals"),
            this,
            menu_selector(GoalsPauseLayer::onGoals)
        );

        goalsButton->setPosition({
            winSize.width / 2.f + 115.f,
            50.f
        });

        auto menu = CCMenu::create();
        menu->addChild(goalsButton);
        menu->setPosition({0.f, 0.f});
        menu->setZOrder(999);

        this->addChild(menu);
    }

    void onGoals(CCObject*) {
        GoalPopup::create()->show();
    }
};
