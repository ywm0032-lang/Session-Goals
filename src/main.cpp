#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

std::vector<std::string> getGoalIds() {
    std::vector<std::string> ids;

    std::string saved = Mod::get()->getSavedValue<std::string>("goal-ids", "");

    if (saved.empty()) {
        return ids;
    }

    std::string current;

    for (char character : saved) {
        if (character == ',') {
            if (!current.empty()) {
                ids.push_back(current);
            }
            current.clear();
        } else {
            current += character;
        }
    }

    if (!current.empty()) {
        ids.push_back(current);
    }

    return ids;
}

bool hasGoalId(std::vector<std::string> const& ids, std::string const& id) {
    for (auto const& existingId : ids) {
        if (existingId == id) {
            return true;
        }
    }

    return false;
}

void saveGoalIds(std::vector<std::string> const& ids) {
    std::string saved;

    for (size_t i = 0; i < ids.size(); i++) {
        if (i > 0) {
            saved += ",";
        }

        saved += ids[i];
    }

    Mod::get()->setSavedValue("goal-ids", saved);
}

class AddGoalPopup : public FLAlertLayer, public FLAlertLayerProtocol {
protected:
    TextInput* m_levelIdInput = nullptr;
    TextInput* m_goalInput = nullptr;

    bool init() {
        if (!FLAlertLayer::init(
            this,
            "",
            "",
            "Cancel",
            "",
            340.f,
            false,
            250.f,
            1.f
        )) {
            return false;
        }

        if (!m_mainLayer || !m_buttonMenu) {
            return false;
        }

        this->setBrownBackground();

        float centerX = 170.f;
        float centerY = 125.f;

        auto titleLabel = CCLabelBMFont::create("Add Goal", "bigFont.fnt");
        if (titleLabel) {
            titleLabel->setScale(0.65f);
            titleLabel->setPosition({centerX, 225.f});
            m_mainLayer->addChild(titleLabel);
        }

        auto levelLabel = CCLabelBMFont::create("Level ID", "bigFont.fnt");
        if (levelLabel) {
            levelLabel->setScale(0.5f);
            levelLabel->setPosition({centerX, centerY + 55.f});
            m_mainLayer->addChild(levelLabel);
        }

        m_levelIdInput = TextInput::create(240.f, "Level ID");
        if (m_levelIdInput) {
            m_levelIdInput->setPosition({centerX, centerY + 25.f});
            m_mainLayer->addChild(m_levelIdInput);
        }

        auto goalLabel = CCLabelBMFont::create("Goal Percentage", "bigFont.fnt");
        if (goalLabel) {
            goalLabel->setScale(0.5f);
            goalLabel->setPosition({centerX, centerY - 20.f});
            m_mainLayer->addChild(goalLabel);
        }

        m_goalInput = TextInput::create(240.f, "Goal %");
        if (m_goalInput) {
            m_goalInput->setPosition({centerX, centerY - 50.f});
            m_mainLayer->addChild(m_goalInput);
        }

        auto saveButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Save"),
            this,
            menu_selector(AddGoalPopup::onSave)
        );

        if (saveButton) {
            saveButton->setPosition({centerX, centerY - 95.f});

            auto menu = CCMenu::create();

            if (menu) {
                menu->addChild(saveButton);
                menu->setPosition({0.f, 0.f});
                m_buttonMenu->addChild(menu);
            }
        }

        return true;
    }

    void setBrownBackground() {
        auto oldBackground = m_mainLayer->getChildByType<CCScale9Sprite>(0);

        auto brownBackground = CCScale9Sprite::create("GJ_square04.png");

        if (!brownBackground) {
            return;
        }

        brownBackground->setContentSize({320.f, 230.f});
        brownBackground->setPosition({170.f, 125.f});
        brownBackground->setZOrder(-1);

        m_mainLayer->addChild(brownBackground);

        if (oldBackground) {
            oldBackground->removeFromParent();
        }
    }

    void onSave(CCObject*) {
        if (!m_levelIdInput || !m_goalInput) {
            return;
        }

        std::string levelId = m_levelIdInput->getString();
        std::string goalText = m_goalInput->getString();

        if (levelId.empty() || goalText.empty()) {
            Notification::create("Enter a level ID and goal.", NotificationIcon::Warning, 2.f)->show();
            return;
        }

        int goal = std::atoi(goalText.c_str());

        if (goal < 1 || goal > 100) {
            Notification::create("Goal must be between 1% and 100%.", NotificationIcon::Warning, 2.f)->show();
            return;
        }

        auto ids = getGoalIds();

        if (!hasGoalId(ids, levelId)) {
            ids.push_back(levelId);
            saveGoalIds(ids);
        }

        Mod::get()->setSavedValue("goal_" + levelId + "_target", goal);
        Mod::get()->setSavedValue("goal_" + levelId + "_best", 0);
        Mod::get()->setSavedValue("goal_" + levelId + "_completed", false);

        Notification::create("Goal added.", NotificationIcon::Success, 2.f)->show();

        this->onBtn1(nullptr);
    }

public:
    static AddGoalPopup* create() {
        auto result = new AddGoalPopup();

        if (result && result->init()) {
            result->autorelease();
            return result;
        }

        if (result) {
            delete result;
        }

        return nullptr;
    }
};

class GoalListPopup : public FLAlertLayer, public FLAlertLayerProtocol {
protected:
    ScrollLayer* m_scrollLayer = nullptr;

    bool init() {
        if (!FLAlertLayer::init(
            this,
            "",
            "",
            "Close",
            "",
            340.f,
            false,
            260.f,
            1.f
        )) {
            return false;
        }

        if (!m_mainLayer || !m_buttonMenu) {
            return false;
        }

        this->setBrownBackground();

        float centerX = 170.f;

        auto titleLabel = CCLabelBMFont::create("Session Goals", "bigFont.fnt");
        if (titleLabel) {
            titleLabel->setScale(0.65f);
            titleLabel->setPosition({centerX, 225.f});
            m_mainLayer->addChild(titleLabel);
        }

        m_scrollLayer = ScrollLayer::create({280.f, 150.f});

        if (m_scrollLayer) {
            m_scrollLayer->setPosition({30.f, 45.f});
            m_mainLayer->addChild(m_scrollLayer);
        }

        this->createPlusButton();
        this->reloadGoals();

        return true;
    }

    void setBrownBackground() {
        auto oldBackground = m_mainLayer->getChildByType<CCScale9Sprite>(0);

        auto brownBackground = CCScale9Sprite::create("GJ_square04.png");

        if (!brownBackground) {
            return;
        }

        brownBackground->setContentSize({320.f, 240.f});
        brownBackground->setPosition({170.f, 130.f});
        brownBackground->setZOrder(-1);

        m_mainLayer->addChild(brownBackground);

        if (oldBackground) {
            oldBackground->removeFromParent();
        }
    }

    void createPlusButton() {
        auto plusSprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");

        if (!plusSprite) {
            return;
        }

        auto plusButton = CCMenuItemSpriteExtra::create(
            plusSprite,
            this,
            menu_selector(GoalListPopup::onAddGoal)
        );

        if (!plusButton) {
            return;
        }

        plusButton->setPosition({295.f, 225.f});
        plusButton->setScale(0.9f);

        auto menu = CCMenu::create();

        if (!menu) {
            return;
        }

        menu->addChild(plusButton);
        menu->setPosition({0.f, 0.f});
        m_buttonMenu->addChild(menu);
    }

    void reloadGoals() {
        if (!m_scrollLayer || !m_scrollLayer->m_contentLayer) {
            return;
        }

        m_scrollLayer->m_contentLayer->removeAllChildren();

        auto ids = getGoalIds();

        if (ids.empty()) {
            auto emptyLabel = CCLabelBMFont::create("No goals yet.
Tap + to add one.", "bigFont.fnt");

            if (emptyLabel) {
                emptyLabel->setScale(0.45f);
                emptyLabel->setPosition({140.f, 75.f});
                m_scrollLayer->m_contentLayer->addChild(emptyLabel);
            }

            m_scrollLayer->m_contentLayer->setContentHeight(150.f);
            return;
        }

        float y = 25.f;

        for (auto const& id : ids) {
            int target = Mod::get()->getSavedValue<int>(
                "goal_" + id + "_target",
                100
            );

            int best = Mod::get()->getSavedValue<int>(
                "goal_" + id + "_best",
                0
            );

            bool completed = Mod::get()->getSavedValue<bool>(
                "goal_" + id + "_completed",
                false
            );

            std::string status = completed
                ? "Completed"
                : "Best: " + std::to_string(best) + "%";

            std::string text = fmt::format("Level {}
Goal: {}%  {}", id, target, status);

            auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");

            if (label) {
                label->setScale(0.4f);
                label->setPosition({140.f, y});
                m_scrollLayer->m_contentLayer->addChild(label);
            }

            y += 50.f;
        }

        m_scrollLayer->m_contentLayer->setContentHeight(y);
    }

    void onAddGoal(CCObject*) {
        AddGoalPopup::create()->show();
    }

public:
    static GoalListPopup* create() {
        auto result = new GoalListPopup();

        if (result && result->init()) {
            result->autorelease();
            return result;
        }

        if (result) {
            delete result;
        }

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

        if (!m_level) {
            return;
        }

        std::string levelId = std::to_string(m_level->m_levelID.value());

        int attempts = Mod::get()->getSavedValue<int>(
            "goal_" + levelId + "_attempts",
            0
        );

        Mod::get()->setSavedValue(
            "goal_" + levelId + "_attempts",
            attempts + 1
        );

        this->updateGoalLabel();
    }

    void updateGoalLabel() {
        if (!m_level) {
            return;
        }

        std::string levelId = std::to_string(m_level->m_levelID.value());

        int target = Mod::get()->getSavedValue<int>(
            "goal_" + levelId + "_target",
            0
        );

        if (target <= 0) {
            return;
        }

        int attempts = Mod::get()->getSavedValue<int>(
            "goal_" + levelId + "_attempts",
            0
        );

        if (!m_fields->m_goalLabel) {
            m_fields->m_goalLabel = CCLabelBMFont::create("", "bigFont.fnt");

            if (!m_fields->m_goalLabel || !m_uiLayer) {
                return;
            }

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
            "Goal: {}%  Attempts: {}",
            target,
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

        if (!goalsButton) {
            return;
        }

        goalsButton->setPosition({
            winSize.width - 110.f,
            50.f
        });

        auto menu = CCMenu::create();

        if (!menu) {
            return;
        }

        menu->addChild(goalsButton);
        menu->setPosition({0.f, 0.f});
        menu->setZOrder(999);

        this->addChild(menu);
    }

    void onGoals(CCObject*) {
        GoalListPopup::create()->show();
    }
};
