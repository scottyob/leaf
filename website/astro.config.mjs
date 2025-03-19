// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

import tailwindcss from '@tailwindcss/vite';

import react from '@astrojs/react';

import icon from 'astro-icon';

import fs from "fs";
import path from "path";

// @ts-ignore
function getMarkdownItems(dir) {
    let results = [];
    let dirMap = {};

    // @ts-ignore
    function readDir(currentDir) {
        const files = fs.readdirSync(currentDir);
        let hasIndex = false;
        let items = [];

        for (const file of files) {
            const fullPath = path.join(currentDir, file);
            const stat = fs.statSync(fullPath);

            if (stat.isDirectory()) {
                readDir(fullPath);
            } else if (file.endsWith(".md") || file.endsWith(".mdx")) {
                let relativePath = fullPath.replace("../docs/", "").replace(/\.(md|mdx)$/, "");
                if (file === "index.md" || file === "index.mdx") {
                    hasIndex = true;
                    relativePath = relativePath.replace(/\/index$/, "");
                }
                items.push(relativePath);
            }
        }

        if (items.length > 0) {
            if (hasIndex && items.length > 1) {
                // @ts-ignore
                dirMap[currentDir] = items;
            } else {
                results.push(...items);
            }
        }
    }

    readDir(dir);

    for (const [key, value] of Object.entries(dirMap)) {
        results.push({
            label: key.split("/").at(-1),
            items: value,
        });
    }

    return results;
}

// https://astro.build/config
export default defineConfig({
    integrations: [starlight({
        title: 'Leaf',
        social: {
            youtube: 'https://www.youtube.com/channel/UCbwWXjxFitbefeKqzAKa94Q',
            instagram: 'https://www.instagram.com/leafvario',
            github: 'https://github.com/DangerMonkeys/leaf',
            discord: 'https://discord.com/channels/1325634753663209472/1325634753663209476',
        },
        sidebar: [
            {
                label: 'User Guides',
                // Use a custom ordering function to allow index files to be in their own directory                
                // @ts-ignore
                items: getMarkdownItems("../docs/user-guides"),
                //autogenerate: { directory: 'user-guides' },
            },
            {
                label: 'Developer Reference',
                // Use a custom ordering function to allow index files to be in their own directory                
                // @ts-ignore
                items: getMarkdownItems("../docs/dev-references"),
                // autogenerate: { directory: 'dev-references' },
            },
            {
                label: 'Legal',
                // Use a custom ordering function to allow index files to be in their own directory                
                // @ts-ignore
                items: getMarkdownItems("../docs/legal"),
                // autogenerate: { directory: 'legal' },
            },
        ],
    }), react(), icon({
        include: {
            "game-icons": [
                'bookmarklet',  // Attribution Required
            ],
            "simple-icons": [
                "github",
                "opensourcehardware",
                "bluetooth",
                "gmail",
                "youtube",
                "instagram",
            ],
            "material-symbols": [
                "bluetooth",
                "wifi-rounded",
                "satellite-alt",
            ]
        },
    })],

    vite: {
        plugins: [tailwindcss()],
        server: {
            allowedHosts: ["sob-desktop.tail2c07b.ts.net", "localhost"]
        }
    },
});