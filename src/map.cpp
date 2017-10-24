#include "map.hpp"

map::map()
: is_loaded(false)
{}

void map::update(b2World* world)
{
    for(auto & o : balls) {
        if(! o->is_alive && ! --(o->respawn_counter)) {
            respawn_ball(o.get());
            o->is_alive = true;
        }

        for(auto & p : o->powerups) {
            --p.counter;
        }

        o->powerups.erase(
            std::remove_if(
                o->powerups.begin(),
                o->powerups.end(),
                [](ball_powerup bp) {
                    return bp.counter != 0;
                }
            ),
            o->powerups.end()
        );

        if(o->should_transport) {
            const portal& p = portals[o->portal_transport_id];
            o->set_position(b2Vec2(p.x, p.y));
            o->should_transport = false;
        }
    }

    for(auto & o : bombs) {
        if(! o.is_alive && ! --o.respawn_counter) o.is_alive = true;
    }

    for(auto & o : powerups) {
        if(! o->is_alive && ! --o->respawn_counter) {
            o->type = o->get_random_type();
            o->is_alive = true;
        }
    }

    for(auto & o : boosters) {
        if(! o.is_alive && ! --o.respawn_counter) o.is_alive = true;
    }
}

void map::respawn_ball(ball* b)
{
    std::cout << "respawning" << std::endl;
    random_util& rng = random_util::get_instance();

    std::vector<spawn> potential_spawns;

    for(auto s : spawns) {
        if(same_color(s.type, b->type)) {
            for(std::size_t i=0; i<(s.weight > 0) ? s.weight : 1; ++i) {
                potential_spawns.emplace_back(s);
            }
        }
    }

    if(potential_spawns.empty()) {
        const spawn_type matching_spawn_type = corresponding_color<spawn_type>(b->type);

        // maps dont require spawn points...
        // but thats stupid
        // maybe this should be fixed in map export
        const flag_type matching_flag_type = corresponding_color<flag_type>(b->type);

        for(auto f : flags) {
            if(f.type == matching_flag_type) {
                potential_spawns.emplace_back(spawn(f.x, f.y, 1, 1, matching_spawn_type));
            }
        }

        // if theres no flags or spawns...
        if(potential_spawns.empty()) {
            std::cerr << "error: no spawns found, placing at 0,0, fix your map" << std::endl;
            potential_spawns.emplace_back(spawn(0, 0, 1, 1, matching_spawn_type));
        }
    }

    const spawn & s = potential_spawns[
        std::uniform_int_distribution<int>(0, potential_spawns.size()-1)(rng.eng)
    ];
    const float a = std::uniform_real_distribution<>(0.0f, TWO_PI)(rng.eng);
    b->set_position(b2Vec2(
        s.x + (std::cos(a) * s.radius),
        s.y + (std::sin(a) + s.radius))
    );
}

ball* map::add_ball(b2World* world, ball b)
{
    std::unique_ptr<ball> o(new ball(b));
    balls.emplace_back(std::unique_ptr<ball>(new ball(b)));
    balls.back()->add_to_world(world);
    respawn_ball(balls.back().get());

    return balls.back().get();
}


b2World * map::init_world()
{
    b2World* world = new b2World(b2Vec2(0, 0));
    static contact_listener contact_listener_instance;
    world->SetContactListener(&contact_listener_instance);

    for(std::size_t i=0; i<8; ++i) {
        add_ball(world, ball(i < 4 ? ball_type::red : ball_type::blue));
    }

    for(auto & m : walls) {
        m.add_to_world(world);
    }

    for(auto & m : spikes) {
        m.add_to_world(world);
    }

    for(auto & m : bombs) {
        m.add_to_world(world);
    }

    for(auto & m : toggles) {
        m.add_to_world(world);
    }

    for(auto & m : boosters) {
        m.add_to_world(world);
    }

    for(auto & m : powerups) {
        m->add_to_world(world);
    }

    for(auto & m : flags) {
        m.add_to_world(world);
    }

    for(auto & m : portals) {
        m.add_to_world(world);
    }

    return world;
}

void to_json(nlohmann::json& j, const map& p)
{
    if(! p.is_loaded) {
        std::cerr << "error: map not loaded" << std::endl;
        j = nlohmann::json{{"error", "map not loaded"}};
        return;
    }

    std::vector<powerup> j_powerups;
    for(auto & o : p.powerups) {
        j_powerups.emplace_back(*o);
    }

    j = nlohmann::json{
        {"meta", {
            {"type",    to_string(p.type)},
            {"name",    p.name},
            {"author",  p.author},
            {"version", p.version},
            {"width",   p.width},
            {"height",  p.height}
        }},
        {"walls",   p.walls},
        {"tiles",   p.tiles},
        {"portals", p.portals},
        {"toggles", p.toggles},
        {"spawns",   p.spawns},
        {"bombs",    p.bombs},
        {"spikes",   p.spikes},
        {"powerups", j_powerups},
        {"boosters", p.boosters},
        {"gates",    p.gates},
        {"flags",    p.flags}
    };
}

void from_json(const nlohmann::json& j, map& p)
{
    if(p.is_loaded) {
        std::cerr << "error: map already loaded" << std::endl;
        return;
    }

    auto meta = j.at("meta");
    p.type    = map_type_from_string(meta.at("type").get<std::string>());
    p.name    = meta.at("name").get<std::string>();
    p.author  = meta.at("author").get<std::string>();
    p.version = meta.at("version").get<int>();
    p.width   = meta.at("width").get<int>();
    p.height  = meta.at("height").get<int>();

    p.walls   = j.at("walls").get<std::vector<wall>>();
    p.tiles   = j.at("tiles").get<std::vector<tile>>();
    p.portals = j.at("portals").get<std::vector<portal>>();

    p.spawns   = j.at("spawns").get<std::vector<spawn>>();
    p.bombs    = j.at("bombs").get<std::vector<bomb>>();
    p.spikes   = j.at("spikes").get<std::vector<spike>>();
    p.toggles  = j.at("toggles").get<std::vector<toggle>>();
    std::vector<powerup> j_powerups = j.at("powerups").get<std::vector<powerup>>();
    for(auto o : j_powerups) {
        p.powerups.emplace_back(new powerup(o));
    }
    p.boosters = j.at("boosters").get<std::vector<booster>>();
    p.gates    = j.at("gates").get<std::vector<gate>>();
    p.flags    = j.at("flags").get<std::vector<flag>>();

    p.is_loaded = true;
}
