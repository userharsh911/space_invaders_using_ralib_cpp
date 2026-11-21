#include <raylib.h>
#include <vector>
#include <iostream>
using namespace std;
#define NUM_FRAMES_PER_LINE     6
#define NUM_LINES               1

class Explosion{
    float frameWidth;
    float frameHeight;
    int currentFrame;
    int currentLine;
    int framesCounter;
    Vector2 position;
    Rectangle frameRec;
    Texture2D texture;
    public:
    Explosion(){}
    Explosion(Vector2 position){
        Image image = LoadImage("./src/images/enemy_explosion.png");
        texture = LoadTextureFromImage(image);
        frameWidth = (float)texture.width/NUM_FRAMES_PER_LINE;
        frameHeight = (float)texture.height/NUM_LINES;
        currentFrame = 0;
        currentLine = 0;
        framesCounter = 0;
        frameRec = { 0, 0, frameWidth, frameHeight };
        this->position = position;
        UnloadImage(image);

    }
    bool compute_animation(){
        framesCounter++;

        if (framesCounter > 2)
        {
            currentFrame++;

            if (currentFrame >= NUM_FRAMES_PER_LINE)
            {
                currentFrame = 0;
                currentLine++;

                if (currentLine >= NUM_LINES)
                {
                    currentLine = 0;
                    return true;
                }
            }

            framesCounter = 0;
        }
        return false;
    }
    void update(){
        frameRec.x = frameWidth*currentFrame;
        frameRec.y = frameHeight*currentLine;
    }
    void draw(){
        DrawTextureRec(texture, frameRec, position, WHITE);
    }
    ~Explosion(){UnloadTexture(texture);}
};
class Game{
    protected:
    int screen_width;
    int screen_height;
    Color color;
    static bool start;
    static bool game_over;
    public:
    vector<Explosion*> explosions;
    Game() : screen_width(1080), screen_height(720), color(BLACK){
        
    }
    void initialize_window(){
        InitWindow(screen_width, screen_height, "Space Invader");
    }
    void bg(){
        ClearBackground(color);
    }
    bool restart(Music &game_music, Sound &game_over_sound){
        if(IsKeyPressed(KEY_R) && game_over){
            StopSound(game_over_sound);
            PlayMusicStream(game_music);
            game_over = false;
            return true;
        }
        return false;
    }
};

class Screen : public Game{
    Texture2D texture;
    Texture2D start_texture;
    Texture2D game_over_texture;
    public:
    Screen(){    
        Image image = LoadImage("./src/images/background.png");
        Image start_image = LoadImage("./src/images/start_screen.png");
        Image game_over_image = LoadImage("./src/images/game_over.png");
        texture = LoadTextureFromImage(image);
        start_texture = LoadTextureFromImage(start_image);
        game_over_texture = LoadTextureFromImage(game_over_image);
        UnloadImage(image); 
        UnloadImage(start_image);    
        UnloadImage(game_over_image);    
    }
    void draw(){
        if(start) DrawTexture(texture, 0, 0,WHITE);
    }
    void start_screen(Sound &start_screen_sound, Music &game_music){
        if(IsKeyPressed(KEY_ENTER) && !start){
            StopSound(start_screen_sound);
            PlayMusicStream(game_music);
            start = true;
        }
        if(!start) DrawTexture(start_texture, 0, 0,WHITE);
    }
    void game_over_screen(Music &game_music){
        if(game_over){
            StopMusicStream(game_music);
            DrawTexture(game_over_texture, 0, 0, WHITE);
        }
    }
    
    ~Screen(){UnloadTexture(texture), UnloadTexture(start_texture), UnloadTexture(game_over_texture);}
};
class Bullet{
    protected:
    float width;
    float height;
    float speed;
    Texture2D texture;
    public:
    Vector2 bullet_pos;
    Bullet(){}
    Bullet(Vector2 pos, int speed, string filename) : width(5), height(20){
        bullet_pos = pos;
        this->speed = speed;
        Image image = LoadImage(&filename[0]);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    void draw(){
        // DrawRectangle
        DrawTexture(texture, bullet_pos.x, bullet_pos.y, WHITE);
    }
    void movement(){
        bullet_pos.y -= speed;
    }
    Vector2 get_dimensions(){
        Vector2 v = {width, height};
        return v;
    }
    ~Bullet(){UnloadTexture(texture);}
    
};

class Player : public Game, public Bullet{
    protected:
    Vector2 pos;
    int width;
    int height;
    float speed;
    bool active;
    Texture2D texture;
    public:
    vector<Bullet*> bullets;
    Player() : width(50), height(50), speed(10){
        pos.x = (screen_width / 2) - (this->width / 2);
        pos.y = screen_height - this->height;
        Image image = LoadImage("./src/images/player.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        active = true;
    }  
    void draw(){
        // DrawRectangle(pos.x, pos.y, width, height,RED);
        DrawTexture(texture, pos.x, pos.y, WHITE);

    }
    void movement(){
        if(IsKeyDown(KEY_D) && active && pos.x <= screen_width - width && start){
            pos.x += speed;
        }else if(IsKeyDown(KEY_A) && active && pos.x >= 0 && start){
            pos.x -= speed;
        }
    }
    void shoot(Sound &player_sound){
        if(IsKeyPressed(KEY_SPACE) && active && start){
            PlaySound(player_sound);
            Bullet* bullet =  new Bullet({pos.x + (width / 2 - 5 / 2.0f), pos.y}, 10, "./src/images/player_bullet.png");
            bullets.push_back(bullet);
        }
    }
    void delete_bullets(){
        for(float i = 0; i < bullets.size(); i++){
            if(bullets[i] && bullets[i]->bullet_pos.y <= -20){
                delete bullets[i];
                bullets[i] = nullptr;
                bullets.erase(bullets.begin() + i);
            }
        }
    }
    bool collision(vector<Bullet*> &bullets, Sound &game_over_sound){
        for(auto& bullet : bullets){
            if(bullet && !game_over && bullet->bullet_pos.y + bullet->get_dimensions().y >= this->pos.y && bullet->bullet_pos.x > this->pos.x - bullet->get_dimensions().x && bullet->bullet_pos.x + bullet->get_dimensions().x < this->pos.x + this->width){
                delete bullet;
                bullet = nullptr;
                PlaySound(game_over_sound);
                active = false;
                game_over = true;
                return true;
            }
        }
        return false;
    }
    void reset(){
        if(IsKeyPressed(KEY_R) && !active && start) active = true;
    }
    ~Player(){UnloadTexture(texture);}
};

class Enemies : public Game , public Bullet, public Explosion{
    static float speed;
    Texture2D texture;
    public:
    int lives;
    float width;
    float height;
    float padding;
    static vector<Bullet*> bullets;
    static int count;
    Vector2 pos;
    Enemies(float x, float y, float lives) : width(50), height(50), padding(30){
        pos.x = x;
        pos.y = y;
        this->lives = lives;
        Image image = LoadImage("./src/images/enemy.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    void set_coordinates(Vector2 pos){this->pos = pos;}
    void draw(){
        // DrawRectangle(pos.x, pos.y, width, height,GREEN);
        DrawTexture(texture, pos.x, pos.y, WHITE);

    }
    void movement(){if(start && !game_over)pos.x += speed;}
    void bounce(){
        if(pos.x >= screen_width - width){
            speed = -abs(speed);
        }else if(pos.x <= 0){
            speed = abs(speed);
        }
    }
    void shoot(Sound &enemy_sound){
        if(start && !game_over){
            PlaySound(enemy_sound);
            Bullet* bullet = new Bullet({pos.x + (width / 2 - 5 / 2.0f), pos.y}, -10, "./src/images/enemy_bullet.png");
            bullets.push_back(bullet);
        }
    }
    bool collision(Player & player, Game& game){
        for(auto& bullet : player.bullets){
            if(bullet && !game_over && bullet->bullet_pos.y <= pos.y + height && bullet->bullet_pos.y >= pos.y && bullet->bullet_pos.x > pos.x - bullet->get_dimensions().x && bullet->bullet_pos.x + bullet->get_dimensions().x < pos.x + width){
                delete bullet;
                bullet = nullptr;
                if(!this->lives){
                    game.explosions.push_back(new Explosion(pos));
                    return true;
                }
                this->lives--;
            }
        }
        return false;
    }
    static void delete_bullets(){
        for(float i = 0; i < bullets.size(); i++){
            if(bullets[i] && bullets[i]->bullet_pos.y + bullets[i]->get_dimensions().y > 720){
                delete bullets[i];
                bullets[i] = nullptr;
                bullets.erase(bullets.begin() + i);
            }
        }
    }
    ~Enemies(){count--;UnloadTexture(texture);}
    
};

float Enemies::speed = 3;
int Enemies::count = 0;
bool Game::start = false;
bool Game::game_over = false;
vector<Bullet*> Enemies::bullets = {};

void create_enemies(vector<Enemies*> &enemies, int row, int col){
    if(enemies.size() > 0) enemies.clear();
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            enemies.push_back(new Enemies(j * 50 + j * 50 , i * 50 + i * 50, row - i));
        }
    }
}

int main(){
    Game game;
    game.initialize_window();
    InitAudioDevice();
    Player player;
    vector<Enemies*> enemies;
    int col = 10;
    int row = 3;
    Enemies::count = col * row;
    create_enemies(enemies, row, col);
    Sound enemy_hit_sound = LoadSound("./src/audio/explosion.mp3");
    Sound start_screen_sound = LoadSound("./src/audio/start_screen.mp3");
    Sound enter_sound = LoadSound("./src/audio/enter.wav");
    Sound player_sound = LoadSound("./src/audio/player_sound.wav");
    Sound enemy_sound = LoadSound("./src/audio/enemy_sound.wav");
    Sound game_over_sound = LoadSound("./src/audio/game_over.mp3");
    Music game_music = LoadMusicStream("./src/audio/game_music.mp3");
    float pan = 1.0f;
    SetMusicPan(game_music, pan);

    float volume = 0.5f;
    SetMusicVolume(game_music, volume);

    PlaySound(start_screen_sound);
    Screen screen;
    SetTargetFPS(60);
    int c = -1;
    int enemy_shoot_count = 0;
    int enemy_shoot_times = 20;
    while(!WindowShouldClose()){
        UpdateMusicStream(game_music);
        player.shoot(player_sound);
        enemy_shoot_count %= enemy_shoot_times;
        if(enemy_shoot_count == enemy_shoot_times - 1) c = GetRandomValue(0, Enemies::count - 1),enemy_shoot_times = GetRandomValue(20, 30), enemy_shoot_count = 0;
        for(unsigned int i = 0; i < enemies.size(); i++){
            if(enemies[i]){
                if(!c)enemies[i]->shoot(enemy_sound);
                c--;
                if(c < 0) break;
            }
        }
        enemy_shoot_count++;
        for(auto &bullets : player.bullets) if(bullets) bullets->movement();
        for(auto &bullets : Enemies::bullets) if(bullets) bullets->movement();
        player.movement();
        player.reset();
        for(auto& enemy : enemies) if(enemy) enemy->movement();
        for(auto& enemy : enemies) if(enemy) enemy->bounce();
        for(auto& enemy : enemies){
            if (enemy && enemy->collision(player, game)) delete enemy, enemy = nullptr, PlaySound(enemy_hit_sound);
        }
        if(Enemies::bullets.size() > 0) player.collision(Enemies::bullets, game_over_sound);
        player.delete_bullets();
        if(Enemies::bullets.size() > 0) Enemies::delete_bullets();
     
        for(auto& explosion : game.explosions){
            if(explosion && explosion->compute_animation()) delete explosion, explosion = nullptr;
        }
        for(auto& explosion : game.explosions) if(explosion) explosion->update();
        if(game.restart(game_music, game_over_sound)) create_enemies(enemies, row, col);
        
        BeginDrawing();
        game.bg();
        screen.draw();
        player.draw();
        for(auto &bullets : player.bullets) if(bullets) bullets->draw();
        for(auto &bullets : Enemies::bullets) if(bullets) bullets->draw();
        for(auto& explosion : game.explosions) if(explosion) explosion->draw();
        for(auto& enemy : enemies) if(enemy) enemy->draw();
        screen.game_over_screen(game_music);
        screen.start_screen(start_screen_sound, game_music);
        EndDrawing();
    }
    UnloadSound(enemy_hit_sound);
    UnloadSound(start_screen_sound);
    UnloadSound(enter_sound);
    UnloadSound(player_sound);
    UnloadSound(game_over_sound);
    UnloadMusicStream(game_music);
    CloseAudioDevice();
    CloseWindow();
}