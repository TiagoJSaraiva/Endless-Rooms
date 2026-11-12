"""Gera os sprites placeholder dos inimigos da Dungeon (Victim, Guard, Executioner).

Como rodar (a partir da raiz do repositório; precisa de Python 3 com Pillow):

    python tools/sprites/dungeon_enemies.py

Escreve em game/assets/img/enemies/dungeon_<inimigo>/:
    idle_sprite.png          um frame (tamanho do frame do inimigo)
    walking_spritesheet.png  4 frames empilhados na vertical
    charge_sprite.png        frame de carga (só Guard e Executioner)
e as armas/projéteis desenhados pelos projéteis:
    dungeon_victim/bone.png, dungeon_guard/spear.png, dungeon_executioner/axe.png

A saída é determinística (sem aleatoriedade): rodar de novo regenera os mesmos bytes.
Os tamanhos de frame precisam bater com `SpriteDefinition` em game/src/enemy_rules.cpp.
As armas são desenhadas com o cabo no topo e a ponta embaixo, na proporção exata em que o
jogo as estica (spear 11x31 -> 22x62, axe 17x65 -> 34x130, bone 8x14 -> 16x28).
Os personagens olham para a direita; o jogo espelha o sprite quando o jogador está à esquerda.
"""

import os
import sys

from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "game", "assets", "img", "enemies")

# ---- Paleta (dados) ----
PALETTE = {
    "bone": (226, 218, 190, 255),
    "bone_shade": (168, 158, 128, 255),
    "socket": (16, 12, 20, 255),
    "outline": (14, 10, 16, 255),
    "iron": (122, 130, 146, 255),
    "iron_light": (176, 184, 198, 255),
    "iron_dark": (76, 82, 96, 255),
    "wood": (112, 76, 42, 255),
    "wood_dark": (78, 50, 28, 255),
    "cloth": (92, 30, 38, 255),
    "cloth_dark": (56, 18, 26, 255),
    "hood": (44, 36, 56, 255),
    "hood_light": (66, 56, 82, 255),
    "glow": (255, 96, 60, 255),
}

# ---- Proporções de cada esqueleto (dados) ----
# Medidas em pixels; o corpo é montado de baixo para cima a partir dos pés.
VICTIM = dict(w=34, h=62, skull_w=12, skull_h=11, torso_w=14, torso_h=15, pelvis_w=10, pelvis_h=3,
              leg_len=22, limb=3, arm_len=17, foot_w=5, body_top_margin=4)
GUARD = dict(w=40, h=70, skull_w=13, skull_h=12, torso_w=16, torso_h=16, pelvis_w=12, pelvis_h=3,
             leg_len=22, limb=3, arm_len=17, foot_w=6, body_top_margin=8)
EXECUTIONER = dict(w=58, h=88, skull_w=16, skull_h=14, torso_w=22, torso_h=20, pelvis_w=16, pelvis_h=4,
                   leg_len=26, limb=5, arm_len=22, foot_w=8, body_top_margin=22)

# Quadros de caminhada: (elevação da perna esquerda, da direita, balanço do braço, afundamento do corpo).
WALK_CYCLE = [(0, 0, 0, 1), (3, 0, 2, 0), (0, 0, 0, 1), (0, 3, -2, 0)]


class Canvas:
    def __init__(self, w, h):
        self.img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        self.px = self.img.load()
        self.w, self.h = w, h

    def rect(self, x, y, w, h, color):
        c = PALETTE[color] if isinstance(color, str) else color
        for yy in range(int(y), int(y + h)):
            for xx in range(int(x), int(x + w)):
                if 0 <= xx < self.w and 0 <= yy < self.h:
                    self.px[xx, yy] = c

    def line(self, x0, y0, x1, y1, color, thick=1):
        # Linha simples por amostragem: suficiente para cabos e lâminas em pixel art.
        steps = max(abs(x1 - x0), abs(y1 - y0), 1)
        for i in range(steps + 1):
            x = round(x0 + (x1 - x0) * i / steps)
            y = round(y0 + (y1 - y0) * i / steps)
            self.rect(x, y, thick, thick, color)

    def outline(self):
        """Contorno escuro de 1 px ao redor das áreas opacas, para ler bem no chão escuro."""
        src = self.img.copy().load()
        c = PALETTE["outline"]
        for y in range(self.h):
            for x in range(self.w):
                if src[x, y][3] != 0:
                    continue
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < self.w and 0 <= ny < self.h and src[nx, ny][3] != 0:
                        self.px[x, y] = c
                        break


def draw_skull(cv, cx, top, p, glow=False, hood=False):
    sw, sh = p["skull_w"], p["skull_h"]
    x = cx - sw // 2
    cv.rect(x, top, sw, sh - 3, "bone")                   # crânio
    cv.rect(x + 1, top - 1, sw - 2, 1, "bone")            # topo arredondado
    cv.rect(x + 2, top + sh - 3, sw - 4, 3, "bone")       # mandíbula mais estreita
    cv.rect(x, top + 3, 1, sh - 7, "bone_shade")          # sombra lateral
    eye = "glow" if glow else "socket"
    ey = top + 3
    cv.rect(cx - 4, ey, 3, 3, eye)                        # órbitas
    cv.rect(cx + 1, ey, 3, 3, eye)
    cv.rect(cx - 1, ey + 4, 2, 2, "socket")               # nariz
    for tx in range(x + 3, x + sw - 3, 2):                # dentes
        cv.rect(tx, top + sh - 2, 1, 2, "socket")
    if hood:
        # Capuz de executor: cobre o topo e as laterais, deixando o rosto à mostra.
        cv.rect(x - 2, top - 3, sw + 4, 5, "hood")
        cv.rect(x - 3, top, 3, sh + 2, "hood")
        cv.rect(x + sw, top, 3, sh + 2, "hood")
        cv.rect(x - 1, top - 2, sw + 2, 1, "hood_light")
        cv.rect(cx - 5, top + 2, 10, 1, "hood")           # sombra sobre a testa


def draw_torso(cv, cx, top, p, armor=False, apron=False):
    tw, th = p["torso_w"], p["torso_h"]
    x = cx - tw // 2
    cv.rect(cx - 1, top - 2, 2, 2, "bone_shade")          # pescoço
    cv.rect(cx - 1, top, 2, th, "bone_shade")             # coluna
    for ry in range(top + 1, top + th - 1, 3):            # costelas
        rw = tw - (0 if ry < top + th // 2 else 2)
        cv.rect(cx - rw // 2, ry, rw, 2, "bone")
        cv.rect(cx - 1, ry, 2, 2, "bone_shade")
    if armor:
        cv.rect(x - 1, top, tw + 2, th - 1, "iron")       # peitoral de ferro
        cv.rect(x - 1, top, tw + 2, 2, "iron_light")
        cv.rect(x - 1, top + th - 3, tw + 2, 2, "iron_dark")
        cv.rect(cx - 1, top + 2, 2, th - 5, "iron_dark")  # friso central
        for rx in (x + 1, x + tw - 2):                    # rebites
            cv.rect(rx, top + 3, 1, 1, "iron_light")
        cv.rect(x - 4, top - 1, 5, 4, "iron")             # ombreiras
        cv.rect(x + tw - 1, top - 1, 5, 4, "iron")
        cv.rect(x - 4, top - 1, 5, 1, "iron_light")
        cv.rect(x + tw - 1, top - 1, 5, 1, "iron_light")
    if apron:
        cv.rect(x - 2, top + th - 2, tw + 4, 2, "wood_dark")  # cinto
        cv.rect(x + 1, top + th, tw - 2, 9, "cloth")          # avental de executor
        cv.rect(x + 1, top + th + 7, tw - 2, 2, "cloth_dark")


def draw_leg(cv, x, top, length, p, lift=0):
    limb = p["limb"]
    length = length - lift
    cv.rect(x, top, limb, length, "bone")
    cv.rect(x, top, 1, length, "bone_shade")
    cv.rect(x - 1, top + length - 1, 1, 1, "bone")        # joelho
    cv.rect(x - 1, top + length, p["foot_w"], 2, "bone")  # pé


def draw_arm(cv, x, top, p, swing=0, bent=False, color="bone"):
    limb, length = p["limb"], p["arm_len"]
    if bent:
        cv.rect(x, top, limb, length // 2, color)
        cv.rect(x, top + length // 2 - 1, limb + 4, limb, color)
    else:
        cv.rect(x, top + max(swing, 0), limb, length - abs(swing), color)
        cv.rect(x, top + length - abs(swing) + max(swing, 0), limb, 2, "bone_shade")  # mão


def draw_body(cv, p, pose, glow=False, armor=False, hood=False, apron=False, crouch=0, wide=0):
    """Esqueleto de frente. `pose` = (perna esq., perna dir., balanço do braço, afundamento)."""
    lift_l, lift_r, swing, bob = pose
    cx = p["w"] // 2
    feet = p["h"] - 3 + 0                                 # topo dos pés fica 2 px acima da base
    bob += crouch
    leg_top = feet - p["leg_len"] - 1
    leg_gap = p["torso_w"] // 4 + 1 + wide
    draw_leg(cv, cx - leg_gap - p["limb"] // 2 - 1, leg_top + bob, p["leg_len"] - bob, p, lift_l)
    draw_leg(cv, cx + leg_gap - p["limb"] // 2 + 1, leg_top + bob, p["leg_len"] - bob, p, lift_r)
    pelvis_top = leg_top + bob - p["pelvis_h"]
    cv.rect(cx - p["pelvis_w"] // 2, pelvis_top, p["pelvis_w"], p["pelvis_h"], "bone_shade")
    torso_top = pelvis_top - p["torso_h"] - 1
    draw_torso(cv, cx, torso_top, p, armor=armor, apron=apron)
    skull_top = torso_top - 2 - p["skull_h"]
    draw_skull(cv, cx, skull_top, p, glow=glow, hood=hood)
    return cx, torso_top, skull_top


def arms_for(cv, p, cx, torso_top, swing, **kw):
    half = p["torso_w"] // 2 + 3
    draw_arm(cv, cx - half - p["limb"] + 1, torso_top + 1, p, swing=swing, **kw)
    draw_arm(cv, cx + half - 1, torso_top + 1, p, swing=-swing, **kw)


# ---------------------------------------------------------------- Victim

def victim_frame(pose):
    p = VICTIM
    cv = Canvas(p["w"], p["h"])
    cx, torso_top, _ = draw_body(cv, p, pose)
    arms_for(cv, p, cx, torso_top, pose[2])
    cv.outline()
    return cv.img


# ---------------------------------------------------------------- Guard

def guard_spear_on_shoulder(cv, p, x):
    """Lança parada, em pé ao lado do corpo (frames de idle e caminhada)."""
    cv.rect(x, 8, 2, p["h"] - 12, "wood")
    cv.rect(x, 8, 1, p["h"] - 12, "wood_dark")
    cv.rect(x - 1, 2, 4, 7, "iron_light")                  # ponta
    cv.rect(x, 0, 2, 3, "iron_light")


def guard_frame(pose, charge=False):
    p = GUARD
    cv = Canvas(p["w"], p["h"])
    cx, torso_top, _ = draw_body(cv, p, pose, glow=charge, armor=True, crouch=2 if charge else 0,
                                 wide=2 if charge else 0)
    if charge:
        # Lança recuada na horizontal, à altura do peito, apontando para a direita.
        y = torso_top + 6
        cv.rect(cx - 16, y, 34, 2, "wood")
        cv.rect(cx + 15, y - 2, 6, 6, "iron_light")
        cv.rect(cx + 20, y - 1, 3, 4, "iron_light")
        arms_for(cv, p, cx, torso_top, 0, bent=True)
    else:
        arms_for(cv, p, cx, torso_top, pose[2])
        guard_spear_on_shoulder(cv, p, cx + p["torso_w"] // 2 + 8)
    cv.outline()
    return cv.img


# ---------------------------------------------------------------- Executioner

def executioner_axe_resting(cv, p, cx, torso_top):
    """Machadão apoiado no ombro: cabo diagonal e lâmina grande atrás da cabeça."""
    sx = cx + p["torso_w"] // 2 + 2
    cv.line(sx - 8, torso_top + 16, sx + 12, 8, "wood", thick=3)
    cv.rect(sx + 6, 2, 12, 16, "iron")                     # lâmina
    cv.rect(sx + 6, 2, 12, 2, "iron_light")
    cv.rect(sx + 15, 4, 3, 12, "iron_dark")
    cv.rect(sx + 16, 6, 2, 8, "iron_light")


def executioner_axe_raised(cv, p, cx, torso_top):
    """Frame de carga: machadão erguido com as duas mãos acima da cabeça."""
    hx = cx + 6
    cv.rect(hx, 4, 3, 40, "wood")
    cv.rect(hx - 10, 2, 11, 16, "iron")                    # lâmina
    cv.rect(hx - 10, 2, 11, 2, "iron_light")
    cv.rect(hx - 10, 4, 3, 12, "iron_dark")
    cv.rect(hx - 9, 6, 2, 8, "iron_light")


def executioner_frame(pose, charge=False):
    p = EXECUTIONER
    cv = Canvas(p["w"], p["h"])
    cx, torso_top, _ = draw_body(cv, p, pose, glow=charge, hood=True, apron=True,
                                 crouch=0, wide=3 if charge else 0)
    if charge:
        executioner_axe_raised(cv, p, cx, torso_top)
        # Braços para cima, segurando o cabo.
        half = p["torso_w"] // 2 + 4
        for sign in (-1, 1):
            ax = cx + sign * half - (p["limb"] // 2)
            cv.rect(ax, torso_top - 4, p["limb"], 14, "bone")
        cv.rect(cx + 4, torso_top - 6, 9, 4, "bone_shade")
    else:
        arms_for(cv, p, cx, torso_top, pose[2])
        executioner_axe_resting(cv, p, cx, torso_top)
    cv.outline()
    return cv.img


# ---------------------------------------------------------------- Armas e projéteis

def make_bone():
    cv = Canvas(8, 14)
    cv.rect(3, 2, 2, 10, "bone")
    cv.rect(3, 2, 1, 10, "bone_shade")
    for x, y in ((1, 0), (4, 0), (1, 11), (4, 11)):        # nós das pontas
        cv.rect(x, y, 3, 3, "bone")
    cv.rect(2, 1, 4, 1, "bone")
    cv.rect(2, 12, 4, 1, "bone")
    return cv.img


def make_spear():
    """Trecho da lança: cabo no topo, ponta de ferro embaixo (11x31)."""
    cv = Canvas(11, 31)
    cv.rect(4, 0, 3, 20, "wood")
    cv.rect(4, 0, 1, 20, "wood_dark")
    cv.rect(3, 18, 5, 2, "iron_dark")                      # encaixe
    cv.rect(2, 20, 7, 6, "iron")
    cv.rect(3, 26, 5, 3, "iron")
    cv.rect(4, 29, 3, 2, "iron_light")
    cv.rect(2, 20, 2, 5, "iron_light")
    return cv.img


def make_axe():
    """Machadão: cabo longo no topo, lâmina grande perto da ponta (17x65)."""
    cv = Canvas(17, 65)
    cv.rect(7, 0, 3, 62, "wood")
    cv.rect(7, 0, 1, 62, "wood_dark")
    cv.rect(6, 40, 5, 3, "iron_dark")                      # encaixe da lâmina
    cv.rect(0, 40, 17, 22, "iron")                         # lâmina larga
    cv.rect(0, 40, 17, 2, "iron_light")
    cv.rect(0, 60, 17, 3, "iron_dark")
    cv.rect(0, 44, 3, 14, "iron_light")
    cv.rect(14, 44, 3, 14, "iron_dark")
    cv.rect(7, 62, 3, 3, "iron_light")                     # contra-ponta
    return cv.img


# ---------------------------------------------------------------- Saída

def save(img, folder, name):
    path = os.path.join(OUT, folder, name)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.save(path, optimize=False)


def sheet(frames):
    w, h = frames[0].size
    out = Image.new("RGBA", (w, h * len(frames)), (0, 0, 0, 0))
    for i, frame in enumerate(frames):
        out.paste(frame, (0, h * i))
    return out


def main():
    idle = (0, 0, 0, 0)
    save(victim_frame(idle), "dungeon_victim", "idle_sprite.png")
    save(sheet([victim_frame(pose) for pose in WALK_CYCLE]), "dungeon_victim", "walking_spritesheet.png")
    save(make_bone(), "dungeon_victim", "bone.png")

    save(guard_frame(idle), "dungeon_guard", "idle_sprite.png")
    save(sheet([guard_frame(pose) for pose in WALK_CYCLE]), "dungeon_guard", "walking_spritesheet.png")
    save(guard_frame(idle, charge=True), "dungeon_guard", "charge_sprite.png")
    save(make_spear(), "dungeon_guard", "spear.png")

    save(executioner_frame(idle), "dungeon_executioner", "idle_sprite.png")
    save(sheet([executioner_frame(pose) for pose in WALK_CYCLE]), "dungeon_executioner", "walking_spritesheet.png")
    save(executioner_frame(idle, charge=True), "dungeon_executioner", "charge_sprite.png")
    save(make_axe(), "dungeon_executioner", "axe.png")
    print("Sprites gerados em", OUT)


if __name__ == "__main__":
    sys.exit(main())
