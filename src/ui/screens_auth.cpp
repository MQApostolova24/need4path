// Log in and Sign up screens.

#include "ui/screens.h"

#include "core/auth.h"
#include "ui/theme.h"

namespace
{
const int CENTER_X = SCREEN_WIDTH / 2;

void showStatus(App& app, const std::string& message, bool isError)
{
    app.statusMessage = message;
    app.statusIsError = isError;
}

void drawStatusMessage(const App& app, float y)
{
    if (!app.statusMessage.empty())
    {
        const Color color = app.statusIsError ? theme::BLOOD_LIGHT : theme::TURQUOISE;
        drawCenteredText(app.statusMessage.c_str(), CENTER_X, static_cast<int>(y), 22, color);
    }
}

void attemptLogin(App& app)
{
    const AuthResponse response = loginUser(USERS_FILE_PATH, app.loginUsername.text, app.loginPassword.text);
    app.loginPassword.text.clear();

    if (!response.success)
    {
        showStatus(app, response.message, true);
        return;
    }

    app.currentUser = response.username;
    app.statusMessage.clear();
    changeScreen(app, Screen::MainMenu);
}

void attemptSignUp(App& app)
{
    const AuthResponse response = registerUser(USERS_FILE_PATH, app.signUpUsername.text, app.signUpPassword.text,
                                               app.signUpConfirm.text, app.rng);
    app.signUpPassword.text.clear();
    app.signUpConfirm.text.clear();

    if (!response.success)
    {
        showStatus(app, response.message, true);
        return;
    }

    // Go back to the login screen with the new name already filled in.
    app.loginUsername.text = response.username;
    app.loginPassword.text.clear();
    app.signUpUsername.text.clear();
    showStatus(app, response.message, false);
    changeScreen(app, Screen::Login);
}
}  // namespace

void runLoginScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawDesertBackground(time);
    drawTitle("TOMB OF THE FORGOTTEN PHARAOH", "Only the worthy may enter the cursed labyrinth", 50);

    const Rectangle panel = {static_cast<float>(CENTER_X) - 260.0f, 180.0f, 520.0f, 450.0f};
    drawPanel(panel);
    drawCenteredText("ENTER THE TOMB", CENTER_X, static_cast<int>(panel.y) + 26, 30, theme::BRIGHT_GOLD);

    const Rectangle userBox = {panel.x + 50.0f, panel.y + 110.0f, panel.width - 100.0f, 46.0f};
    const Rectangle passBox = {panel.x + 50.0f, panel.y + 200.0f, panel.width - 100.0f, 46.0f};
    TextField* fields[] = {&app.loginUsername, &app.loginPassword};
    const Rectangle boxes[] = {userBox, passBox};
    updateTextFields(fields, boxes, 2);
    drawTextField(app.loginUsername, userBox);
    drawTextField(app.loginPassword, passBox);

    bool submit = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    const float halfWidth = (panel.width - 110.0f) / 2.0f;

    if (drawButton({panel.x + 50.0f, panel.y + 280.0f, panel.width - 100.0f, 52.0f}, "Log In", false))
    {
        submit = true;
    }
    if (drawButton({panel.x + 50.0f, panel.y + 352.0f, halfWidth, 46.0f}, "Sign Up", false))
    {
        app.statusMessage.clear();
        changeScreen(app, Screen::SignUp);
        return;
    }
    if (drawButton({panel.x + 60.0f + halfWidth, panel.y + 352.0f, halfWidth, 46.0f}, "Quit", false))
    {
        app.screen = Screen::Exit;
        return;
    }

    drawStatusMessage(app, panel.y + panel.height + 18.0f);
    drawCenteredText("Tab - next field     Enter - log in", CENTER_X, SCREEN_HEIGHT - 40, 18, theme::INK);

    if (submit)
    {
        attemptLogin(app);
    }
}

void runSignUpScreen(App& app)
{
    const float time = static_cast<float>(GetTime());
    drawDesertBackground(time);
    drawTitle("JOIN THE EXPEDITION", "Carve your name into the tomb's register", 40);

    const Rectangle panel = {static_cast<float>(CENTER_X) - 280.0f, 160.0f, 560.0f, 560.0f};
    drawPanel(panel);
    drawCenteredText("CREATE AN ACCOUNT", CENTER_X, static_cast<int>(panel.y) + 24, 30, theme::BRIGHT_GOLD);

    const float boxX = panel.x + 50.0f;
    const float boxWidth = panel.width - 100.0f;
    const Rectangle userBox = {boxX, panel.y + 100.0f, boxWidth, 46.0f};
    const Rectangle passBox = {boxX, panel.y + 185.0f, boxWidth, 46.0f};
    const Rectangle confirmBox = {boxX, panel.y + 270.0f, boxWidth, 46.0f};

    TextField* fields[] = {&app.signUpUsername, &app.signUpPassword, &app.signUpConfirm};
    const Rectangle boxes[] = {userBox, passBox, confirmBox};
    updateTextFields(fields, boxes, 3);
    drawTextField(app.signUpUsername, userBox);
    drawTextField(app.signUpPassword, passBox);
    drawTextField(app.signUpConfirm, confirmBox);

    const int hintX = static_cast<int>(boxX);
    DrawText("Username: 3-16 letters, digits or _, starting with a letter", hintX,
             static_cast<int>(panel.y) + 330, 16, theme::SAND);
    DrawText("Password: 6-24 characters with a letter and a digit", hintX,
             static_cast<int>(panel.y) + 352, 16, theme::SAND);

    bool submit = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    if (drawButton({boxX, panel.y + 395.0f, boxWidth, 52.0f}, "Create Account", false))
    {
        submit = true;
    }
    if (drawButton({boxX, panel.y + 465.0f, boxWidth, 46.0f}, "Back to Log In", false) || IsKeyPressed(KEY_ESCAPE))
    {
        app.statusMessage.clear();
        changeScreen(app, Screen::Login);
        return;
    }

    drawStatusMessage(app, panel.y + panel.height + 14.0f);

    if (submit)
    {
        attemptSignUp(app);
    }
}
